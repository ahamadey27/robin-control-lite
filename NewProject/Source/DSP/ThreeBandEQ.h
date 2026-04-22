/*
  ==============================================================================

    ThreeBandEQ.h
    3-Band Equalizer Processor for Round Robin Lite

    Provides low shelf, mid peak, and high shelf filtering.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * ThreeBandEQ - Three-band equalizer processor
 *
 * Uses IIR filters to provide:
 * - Low shelf (boost/cut below frequency)
 * - Mid peak/bell (boost/cut around center frequency)
 * - High shelf (boost/cut above frequency)
 *
 * Processing order: Low → Mid → High
 */
class ThreeBandEQ
{
public:
    ThreeBandEQ();
    ~ThreeBandEQ() = default;

    /**
     * Initialize the EQ with sample rate and buffer size
     * Must be called before processBlock()
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock);

    /**
     * Update all filter coefficients based on parameter values
     *
     * @param lowGain_dB    Low shelf gain in dB (-24 to +24)
     * @param lowFreq_Hz    Low shelf cutoff in Hz (20 to 500)
     * @param midGain_dB    Mid peak gain in dB (-24 to +24)
     * @param midFreq_Hz    Mid peak center frequency in Hz (200 to 5000)
     * @param highGain_dB   High shelf gain in dB (-24 to +24)
     * @param highFreq_Hz   High shelf cutoff in Hz (2000 to 20000)
     */
    void updateFilters(float lowGain_dB, float lowFreq_Hz,
        float midGain_dB, float midFreq_Hz,
        float highGain_dB, float highFreq_Hz);

    /**
     * Process audio buffer through all three EQ bands
     * Applies filters in series: Low → Mid → High
     */
    void processBlock(juce::AudioBuffer<float>& buffer);

    /**
     * Reset filter states (call when playback stops)
     */
    void reset();

private:
    // IIR filter types for each band
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;

    // Stereo filter chains (left/right channels)
    juce::dsp::ProcessorDuplicator<Filter, FilterCoefs> lowShelfFilter;
    juce::dsp::ProcessorDuplicator<Filter, FilterCoefs> midPeakFilter;
    juce::dsp::ProcessorDuplicator<Filter, FilterCoefs> highShelfFilter;

    // Processing spec for JUCE DSP
    juce::dsp::ProcessSpec spec;
    double currentSampleRate = 44100.0;
    bool isPrepared = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreeBandEQ)
};
