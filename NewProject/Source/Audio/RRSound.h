/*
  ==============================================================================

    Custom SynthesiserSound class for Round Robin Lite.
    Represents a single loaded audio sample with MIDI note mapping.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../Data/SampleSlot.h"

//==============================================================================
/**
 * RRSound - Represents a loaded audio sample for the Round Robin Lite sampler.
 *
 * IMPORTANT: Round Robin Lite plays samples UNPITCHED.
 * All samples play at their original pitch regardless of which MIDI key is pressed.
 *
 * Each RRSound stores:
 * - The actual audio data (mono buffer)
 * - Which key pair slot it belongs to (for UI organization)
 * - Display information for the UI
 */

class RRSound : public juce::SynthesiserSound
{
public:
    // Constructor & Destructor
    RRSound();
    ~RRSound() override;

    /**
     * Determines if this sound should play when the given MIDI note is triggered.
     *
     * Round Robin Lite: Samples play UNPITCHED across ALL MIDI keys.
     * This function returns true for ALL MIDI notes (0-127).
     *
     * @param midiNoteNumber The MIDI note number (0-127)
     * @return true for all valid MIDI notes
     */
    bool appliesToNote(int midiNoteNumber) override;

    /**
     * Determines if this sound responds to the given MIDI channel.
     * For Round Robin Lite, we respond to all channels (1-16).
     * @param midiChannel The MIDI channel (1-16)
     * @return true (always responds to all channels)
     */
    bool appliesToChannel(int midiChannel) override;

    //==============================================================================
    // Audio Data Management

    /**
     * Loads an audio file into this sound's buffer.
     *
     * This method:
     * - Reads the audio file using JUCE's AudioFormatManager
     * - Converts stereo to mono (if needed)
     * - Stores the original sample rate
     * - Stores the audio data in our buffer
     * @param file The audio file to load
     * @param formatManager Reference to the audio format manager (for reading files)
     * @return true if loading was successful, false otherwise
     */
    bool loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager);
    void setFromSlot(const SampleSlot& slot);

    /**
    * Gets a reference to the audio buffer containing the sample data.
    * This is used by RRVoice to actually play the sound.
    * @return Const reference to the audio buffer
    */
    const juce::AudioBuffer<float>& getAudioBuffer() const { return audioBuffer; }

    /**
     * Gets the number of samples in the loaded audio buffer.
     * @return Number of samples, or 0 if no audio is loaded
     */
    int getNumSamples() const { return audioBuffer.getNumSamples(); }

    /**
     * Gets the original sample rate of the loaded audio file.
     * @return The sample rate (e.g., 44100.0)
     */
    double getOriginalSampleRate() const { return originalSampleRate; }

    //==============================================================================
    // Key Pair Assignment (for UI organization only - does NOT affect pitch)

    /**
     * Sets which key pair slot this sound is assigned to (0-9).
     * This is used ONLY for UI organization, NOT for pitch mapping.
     * @param pairIndex The key pair index (0-9)
     */
    void setKeyPairIndex(int pairIndex);

    /**
     * Gets the current key pair index.
     * @return The key pair index (0-9, or -1 if not assigned)
     */
    int getKeyPairIndex() const { return keyPairIndex; }

    /**
     * Gets the root MIDI note number (C1 = MIDI 36).
     * This is a reference value only - it does NOT affect playback pitch.
     * @return The root MIDI note number (always 36 for C1)
     */
    int getRootNote() const { return rootNote; }

    //==============================================================================
    // Display Information

    /**
     * Sets the display name for this sample (shown in UI).
     * @param name The display name (e.g., "FootstepLeft.wav")
     */
    void setDisplayName(const juce::String& name) { displayName = name; }

    /**
    * Gets the display name.
    * @return The display name
    */
    juce::String getDisplayName() const { return displayName; }

    /**
     * Checks if this sound has audio data loaded.
     * @return true if audio buffer contains samples
     */
    bool isLoaded() const { return audioBuffer.getNumSamples() > 0; }

    /**
     * Clears all sample data and resets to empty state.
     */
    void clearSample();

private:
    //==============================================================================
    // Member Variables

    juce::AudioBuffer<float> audioBuffer;    // The actual audio sample data (mono)
    double originalSampleRate = 44100.0;     // Sample rate of the loaded file
    int rootNote = 36;                       // C1 (MIDI 36) reference pitch
    juce::String displayName;                // Name to show in UI
    int keyPairIndex = -1;                   // Which key pair slot (0-9), -1 = unassigned

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RRSound)
};