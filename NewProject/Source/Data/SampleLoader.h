#pragma once
#include <JuceHeader.h>
#include "SampleSlot.h"
#include "../Audio/RRSound.h"

/**
 * SampleLoader - Coordinates loading files into SampleSlots
 * and keeping the Synthesiser sounds in sync.
 */
class SampleLoader
{
public:
    SampleLoader(juce::AudioFormatManager&formatManager,
                 juce::Synthesiser&synthesiser,
                 SampleSlot*slots,
                 int numSlots);

    const juce::String& getLastError() const { return lastErrorMessage; }

    /**
     * Loads a file into the given slot index and refreshes synthesiser sounds.
     * @return true on success
     */
    bool loadSample(int slotIndex, const juce::File& file);

    /** Clears a slot and refreshes synthesiser sounds. */
    void clearSlot(int slotIndex);

    /** Rebuilds all synthesiser sounds from current slot state. */
    void updateSynthesiserSounds();

    void setSampleRate(double newSampleRate);

private:
    juce::AudioFormatManager&formatManager;
    juce::Synthesiser& synthesiser;
    SampleSlot* slots;
    int numSlots;
    juce::String lastErrorMessage;
    double currentSampleRate = 44100.0;

    /** Resamples a buffer from sourceSampleRate to currentSampleRate in-place. */
    void resampleBuffer(juce::AudioBuffer<float>& buffer, double sourceSampleRate);
};