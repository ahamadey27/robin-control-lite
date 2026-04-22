#include "RRSound.h"
#include "MidiMapper.h"

//==============================================================================
// Constructor
RRSound::RRSound()
{
    // Initialize with default values
    // audioBuffer starts empty (0 channels, 0 samples)
}

// Destructor
RRSound::~RRSound()
{
    // audioBuffer will clean itself up automatically
}

//==============================================================================
// JUCE SynthesiserSound Required Methods

bool RRSound::appliesToNote(int midiNoteNumber)
{
    // Round Robin Lite plays samples UNPITCHED across ALL keys
    // Any MIDI note (0-127) will trigger this sample
    // The sample always plays at its original pitch

    // Validate MIDI note range
    if (midiNoteNumber < 0 || midiNoteNumber > 127)
        return false;

    // Accept ALL valid MIDI notes
    return true;
}

bool RRSound::appliesToChannel(int midiChannel)
{
    // Respond to all MIDI channels (1-16)
    return true;
}

//==============================================================================
// Audio Loading

bool RRSound::loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
    if (!file.existsAsFile())
    {
        DBG("File does not exist: " + file.getFullPathName());
        return false;
    }

    // Try to create a reader for the audio file
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
    {
        DBG("Failed to create reader for file: " + file.getFullPathName());
        return false;
    }

    // Store the original sample rate
    originalSampleRate = reader->sampleRate;

    // Prepare buffer for mono audio
    const int numSamples = static_cast<int>(reader->lengthInSamples);
    audioBuffer.setSize(1, numSamples);  // 1 channel (mono)

    // Read audio file into buffer
    if (reader->numChannels == 1)
    {
        // Mono source - read directly
        reader->read(&audioBuffer, 0, numSamples, 0, true, false);
    }
    else
    {
        // Stereo (or more) - mix to mono
        juce::AudioBuffer<float> tempBuffer(reader->numChannels, numSamples);
        reader->read(&tempBuffer, 0, numSamples, 0, true, true);

        // Mix all channels to mono (simple average)
        audioBuffer.clear();
        for (int channel = 0; channel < reader->numChannels; ++channel)
        {
            audioBuffer.addFrom(0, 0, tempBuffer, channel, 0, numSamples,
                1.0f / reader->numChannels);
        }
    }

    // Set display name from filename (without extension)
    displayName = file.getFileNameWithoutExtension();

    DBG("Successfully loaded: " + displayName +
        " (" + juce::String(numSamples) + " samples at " +
        juce::String(originalSampleRate) + " Hz)");

    return true;
}

//==============================================================================
// Key Pair Assignment

void RRSound::setKeyPairIndex(int pairIndex)
{
    // Validate using MidiMapper
    if (!MidiMapper::isValidPairIndex(pairIndex))
    {
        DBG("Invalid key pair index: " + juce::String(pairIndex));
        keyPairIndex = -1;
        return;
    }

    keyPairIndex = pairIndex;

    // Set root note to C1 (MIDI 36) - the reference pitch
    // Note: In unpitched mode, rootNote doesn't affect playback,
    // but we store it for UI display and future features
    rootNote = MidiMapper::ROOT_MIDI_NOTE;

    DBG("Sound assigned to key pair " + juce::String(pairIndex) +
        " (" + MidiMapper::getKeyPairName(pairIndex) + ")" +
        " | Root note: C1 (MIDI 36) - UNPITCHED PLAYBACK");
}

//==============================================================================
// Helper Methods

void RRSound::clearSample()
{
    audioBuffer.setSize(0, 0);
    displayName = "";
    keyPairIndex = -1;
    rootNote = 36;  // C1 (MIDI 36) - Changed from 48
}
