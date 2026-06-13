#pragma once
#include <JuceHeader.h>

/**
 * SampleSlot - Holds metadata and audio data for one of the 20 sample slots.
 * Tracks file location, display name, and loaded state for UI and preset management.
 * The actual JUCE synthesizer sound (RRSound) is kept in the Synthesiser separately.
 */
struct SampleSlot
{
    //==============================================================================
    // Data Members

    juce::AudioBuffer<float> audioBuffer;   // Raw audio data (mono)
    double sampleRate = 44100.0; // Set this when loading the file
    bool isLoaded = false;
    juce::File sourceFile;    // Original file location
    juce::String displayName;   // Filename shown in UI (no extension)

    //==============================================================================
    // Methods

    /** Resets slot to empty state. */
    void clear()
    {
        audioBuffer.setSize(0, 0);
        sampleRate = 44100.0;
        isLoaded = false;
        sourceFile = juce::File();
        displayName = juce::String();
    }

    /**
     * Loads an audio file into this slot.
     * Handles stereo-to-mono conversion automatically.
     * @return true on success
     */
    bool loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager)
    {
        clear();

        if (!file.existsAsFile())
            return false;

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

        if (reader == nullptr)
            return false;

        // Guard the int64 -> int narrowing below. A corrupt or crafted header can
        // report a decoded length far beyond the file's actual size — SampleLoader's
        // 200 MB cap is on FILE bytes, not decoded length, and compressed formats
        // carry the length in metadata that can lie. Without this, a value past
        // INT_MAX wraps negative and setSize() would assert/misbehave, and an
        // enormous-but-positive value would OOM the (numChannels x numSamples) read
        // buffer. 50M frames mirrors the 200 MB file cap (200 MB / 4 bytes-per-float)
        // — far longer than any real one-shot/loop this sampler loads.
        constexpr juce::int64 maxDecodedSamples = 50000000LL;
        if (reader->lengthInSamples <= 0 || reader->lengthInSamples > maxDecodedSamples)
            return false;

        const int numSamples = static_cast<int>(reader->lengthInSamples);
        sampleRate = reader->sampleRate;

        if (reader->numChannels == 1)
        {
            audioBuffer.setSize(1, numSamples);
            reader->read(&audioBuffer, 0, numSamples, 0, true, false);
        }
        else
        {
            // Mix all channels to mono
            juce::AudioBuffer<float> temp(reader->numChannels, numSamples);
            reader->read(&temp, 0, numSamples, 0, true, true);

            audioBuffer.setSize(1, numSamples);
            audioBuffer.clear();
            for (int ch = 0; ch < reader->numChannels; ++ch)
                audioBuffer.addFrom(0, 0, temp, ch, 0, numSamples, 1.0f / reader->numChannels);
        }

        sourceFile = file;
        displayName = file.getFileNameWithoutExtension();
        isLoaded = true;

        return true;
    }
};