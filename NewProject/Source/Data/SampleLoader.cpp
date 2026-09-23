#include "SampleLoader.h"

SampleLoader::SampleLoader(juce::AudioFormatManager& formatManager,
    juce::Synthesiser& synthesiser,
    SampleSlot* slots,
    int          numSlots,
    const juce::CriticalSection& callbackLock)
    : formatManager(formatManager),
    synthesiser(synthesiser),
    slots(slots),
    numSlots(numSlots),
    callbackLock(callbackLock)
{
}

bool SampleLoader::loadSample(int slotIndex, const juce::File& file)
{
    if (slotIndex < 0 || slotIndex >= numSlots)
    {
        DBG("SampleLoader: Invalid slot index: " + juce::String(slotIndex));
        return false;
    }

    // --- Error Check 1: File existence ---
    if (!file.existsAsFile())
    {
        DBG("SampleLoader: File not found: " + file.getFullPathName());
        lastErrorMessage = "File not found: " + file.getFileName();
        return false;
    }

    // --- Error Check 2: File size limit (200MB) ---
    const int64_t maxFileSizeBytes = 200LL * 1024 * 1024;
    if (file.getSize() > maxFileSizeBytes)
    {
        DBG("SampleLoader: File too large (" + juce::String(file.getSize() / (1024 * 1024)) + "MB): " + file.getFileName());
        lastErrorMessage = "File too large (max 200MB): " + file.getFileName();
        return false;
    }

    // --- Error Check 3: Supported format ---
    auto extension = file.getFileExtension().toLowerCase();
    juce::StringArray supportedFormats = { ".wav", ".aif", ".aiff", ".flac", ".ogg", ".mp3" };
    if (!supportedFormats.contains(extension))
    {
        DBG("SampleLoader: Unsupported format '" + extension + "': " + file.getFileName());
        lastErrorMessage = "Unsupported format '" + extension + "'. Use WAV, AIFF, FLAC, OGG, or MP3.";
        return false;
    }

    // --- Error Check 4: Readable by format manager (catches corrupted files) ---
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        DBG("SampleLoader: Could not read file (corrupted or truly unsupported): " + file.getFileName());
        lastErrorMessage = "Could not read file - it may be corrupted: " + file.getFileName();
        return false;
    }

    // --- Error Check 5: Unsupported sample rate ---
    const double minSampleRate = 8000.0;
    const double maxSampleRate = 192000.0;
    if (! std::isfinite(reader->sampleRate)
        || reader->sampleRate < minSampleRate || reader->sampleRate > maxSampleRate)
    {
        DBG("SampleLoader: Unsupported sample rate (" + juce::String(reader->sampleRate) + "Hz): " + file.getFileName());
        lastErrorMessage = "Unsupported sample rate: " + file.getFileName();
        return false;
    }

    // --- Attempt load into slot ---
    // loadFromFile creates and validates its own reader before allocating.
    reader.reset();

    // Decode + resample on the calling thread without holding the audio lock —
    // big files can take tens of ms and we don't want to stall processBlock.
    SampleSlot pending;
    bool success = pending.loadFromFile(file, formatManager);

    if (success)
    {
        if (pending.audioBuffer.getNumSamples() == 0 ||
            pending.audioBuffer.getNumChannels() == 0)
        {
            DBG("SampleLoader: Buffer empty after load - possible memory issue: " + file.getFileName());
            lastErrorMessage = "Failed to allocate audio buffer for: " + file.getFileName();
            return false;
        }

        double targetSampleRate;
        {
            const juce::ScopedLock sl(callbackLock);
            targetSampleRate = currentSampleRate;
        }
        if (! resampleBuffer(pending.audioBuffer, pending.sampleRate, targetSampleRate))
        {
            lastErrorMessage = "Sample is too large at the current playback rate: " + file.getFileName();
            return false;
        }
        pending.sampleRate = targetSampleRate;

        // Hand the prepared slot to the audio side under the callback lock.
        {
            const juce::ScopedLock sl(callbackLock);
            if (! juce::exactlyEqual(targetSampleRate, currentSampleRate))
            {
                lastErrorMessage = "Playback rate changed while loading; please load the sample again.";
                return false;
            }
            slots[slotIndex] = std::move(pending);
        }

        lastErrorMessage.clear();
        DBG("Slot " + juce::String(slotIndex) + " loaded: " + file.getFileName());
    }
    else
    {
        // loadFromFile failed - fallback: ensure slot is clean
        {
            const juce::ScopedLock sl(callbackLock);
            slots[slotIndex].clear();
        }
        lastErrorMessage = "Failed to load: " + file.getFileName();
        DBG("Slot " + juce::String(slotIndex) + " failed to load: " + file.getFullPathName());
    }

    return success;
}

void SampleLoader::clearSlot(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= numSlots)
        return;

    {
        const juce::ScopedLock sl(callbackLock);
        slots[slotIndex].clear();
    }
    DBG("SampleLoader: Slot " + juce::String(slotIndex) + " cleared");
}


void SampleLoader::setSampleRate(double newSampleRate)
{
    if (! std::isfinite(newSampleRate) || newSampleRate <= 0.0)
        return;

    // Called during prepareToPlay, while the host has stopped processing.
    // Serialize this lifecycle mutation with sample loading/state restoration.
    const juce::ScopedLock sl(callbackLock);
    if (juce::exactlyEqual(newSampleRate, currentSampleRate))
        return;

    currentSampleRate = newSampleRate;
    DBG("SampleLoader: Sample rate set to " + juce::String(newSampleRate));

    for (int i = 0; i < numSlots; ++i)
    {
        if (!slots[i].isLoaded)
            continue;

        juce::AudioBuffer<float> resampled;
        resampled.makeCopyOf(slots[i].audioBuffer);
        if (! resampleBuffer(resampled, slots[i].sampleRate, currentSampleRate))
        {
            slots[i].clear();
            lastErrorMessage = "A sample exceeded the decoded size limit at the new playback rate.";
            continue;
        }

        slots[i].audioBuffer = std::move(resampled);
        slots[i].sampleRate = currentSampleRate;
    }

}

bool SampleLoader::resampleBuffer(juce::AudioBuffer<float>& buffer, double sourceSampleRate,
                                  double targetSampleRate)
{
    if (! std::isfinite(sourceSampleRate) || sourceSampleRate <= 0.0
        || ! std::isfinite(targetSampleRate) || targetSampleRate <= 0.0)
        return false;
    if (juce::exactlyEqual(sourceSampleRate, targetSampleRate))
        return true;

    const double ratio = sourceSampleRate / targetSampleRate;
    const int numChannels = buffer.getNumChannels();
    const int originalNumSamples = buffer.getNumSamples();
    const double length = (double) originalNumSamples / ratio;
    if (! std::isfinite(length) || length <= 0.0 || length > SampleSlot::maxDecodedSamples)
        return false;
    const int resampledNumSamples = juce::jmax(1, juce::roundToInt(length));

    juce::AudioBuffer<float> resampledBuffer(numChannels, resampledNumSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        juce::LagrangeInterpolator interpolator;
        interpolator.reset();

        const float* source = buffer.getReadPointer(ch);
        float* dest = resampledBuffer.getWritePointer(ch);

        interpolator.process(ratio, source, dest, resampledNumSamples, originalNumSamples, 0);
    }

    buffer = std::move(resampledBuffer);

    DBG("SampleLoader: Resampled " + juce::String(originalNumSamples) + " -> "
        + juce::String(resampledNumSamples) + " samples ("
        + juce::String(sourceSampleRate) + "Hz -> "
        + juce::String(targetSampleRate) + "Hz)");
    return true;
}


void SampleLoader::updateSynthesiserSounds()
{
    const juce::ScopedLock sl(callbackLock);

    // Update existing sound in place instead of clearing
    if (synthesiser.getNumSounds() > 0)
    {
        if (auto* sound = dynamic_cast<RRSound*>(synthesiser.getSound(0).get()))
        {
            // Find first loaded slot and update sound
            for (int i = 0; i < numSlots; ++i)
            {
                if (slots[i].isLoaded)
                {
                    sound->setFromSlot(slots[i]);
                    DBG("Sound updated from slot " + juce::String(i));
                    return;
                }
            }

            // No loaded slots — clear the sound so nothing plays
            sound->clearSample();
            DBG("All slots empty — sound cleared");
        }
    }
}
