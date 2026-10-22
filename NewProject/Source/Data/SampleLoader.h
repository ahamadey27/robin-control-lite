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
                 int numSlots,
                 const juce::CriticalSection& callbackLock);

    const juce::String& getLastError() const { return lastErrorMessage; }

    /**
     * Low-level slot-only load. UI callers use the processor's pool APIs so
     * playback metadata is committed in the same callback-lock transaction.
     * @return true on success
     */
    bool loadSample(int slotIndex, const juce::File& file);

    /** Decode/resample without changing live slots; safe to do outside the callback lock. */
    bool prepareSample(const juce::File& file, SampleSlot& pending);

    /** Slot-only commit if the rate is current; caller coordinates pool metadata. */
    bool commitSample(int slotIndex, SampleSlot& pending);

    /** Low-level slot-only clear; caller coordinates pool metadata. */
    void clearSlot(int slotIndex);

    /** Rebuilds all synthesiser sounds from current slot state. */
    void updateSynthesiserSounds();

    void setSampleRate(double newSampleRate);

private:
    juce::AudioFormatManager&formatManager;
    juce::Synthesiser& synthesiser;
    SampleSlot* slots;
    int numSlots;
    const juce::CriticalSection& callbackLock;
    juce::String lastErrorMessage;
    double currentSampleRate = 44100.0;

    /** Resamples a buffer from sourceSampleRate to currentSampleRate in-place. */
    static bool resampleBuffer(juce::AudioBuffer<float>& buffer, double sourceSampleRate,
                               double targetSampleRate);
};
