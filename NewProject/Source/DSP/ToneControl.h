#pragma once
#include <JuceHeader.h>

class ToneControl
{
public:
    ToneControl();
    ~ToneControl() = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void updateFilters(float lowGain_dB, float highGain_dB);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void reset();

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;

    juce::dsp::ProcessorDuplicator<Filter, FilterCoefs> lowShelfFilter;
    juce::dsp::ProcessorDuplicator<Filter, FilterCoefs> highShelfFilter;

    juce::dsp::ProcessSpec spec;
    double currentSampleRate = 44100.0;
    bool isPrepared = false;

    static constexpr float lowFreqHz  = 250.0f;
    static constexpr float highFreqHz = 4000.0f;
    static constexpr float Q          = 0.707f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToneControl)
};
