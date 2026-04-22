/*
  ==============================================================================

    TransientShaper.h
    Transient Master Processor for Round Robin Lite

    Enhances or reduces attack/sustain portions of audio.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * TransientShaper - Attack/Decay transient processor
 *
 * Separates audio into fast (transient) and slow (sustain) components,
 * then applies independent gain control to each.
 *
 * Parameter range: -127 to +127
 * - Negative values: Reduce component (softer)
 * - 0: No change (unity gain)
 * - Positive values: Enhance component (louder)
 */
class TransientShaper
{
public:
    TransientShaper();
    ~TransientShaper() = default;

    /**
     * Initialize with sample rate and buffer size
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock);

    /**
     * Process audio buffer with transient shaping
     *
     * @param buffer        Audio buffer to process (modified in-place)
     * @param attackAmount  Attack enhancement (-127 to +127)
     * @param decayAmount   Sustain enhancement (-127 to +127)
     */
    void processBlock(juce::AudioBuffer<float>& buffer, float attackAmount, float decayAmount);

    /**
     * Reset envelope follower state
     */
    void reset();

private:
    /**
     * Convert parameter value (-127 to +127) to gain multiplier
     * -127 = 0.5x (half volume)
     * 0 = 1.0x (no change)
     * +127 = 2.0x (double volume)
     */
    float parameterToGain(float paramValue);

    /**
     * Detect envelope of audio signal
     * Returns smoothed amplitude curve
     */
    float detectEnvelope(float input, float& envelopeState, bool isFast);

    // Envelope follower states (one per channel)
    float fastEnvelopeL = 0.0f;
    float fastEnvelopeR = 0.0f;
    float slowEnvelopeL = 0.0f;
    float slowEnvelopeR = 0.0f;

    // Time constants for envelope detection
    float fastAttack = 0.0f;
    float fastRelease = 0.0f;
    float slowAttack = 0.0f;
    float slowRelease = 0.0f;

    double currentSampleRate = 44100.0;
    bool isPrepared = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransientShaper)
};