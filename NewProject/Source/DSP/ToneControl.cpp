#include "ToneControl.h"

ToneControl::ToneControl()
{
}

void ToneControl::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    lowShelfFilter.prepare(spec);
    highShelfFilter.prepare(spec);

    updateFilters(0.0f, 0.0f);

    isPrepared = true;
}

void ToneControl::updateFilters(float lowGain_dB, float highGain_dB)
{
    if (!isPrepared)
        return;

    lowGain_dB  = juce::jlimit(-12.0f, 12.0f, lowGain_dB);
    highGain_dB = juce::jlimit(-12.0f, 12.0f, highGain_dB);

    *lowShelfFilter.state = *FilterCoefs::makeLowShelf(
        currentSampleRate, lowFreqHz, Q,
        juce::Decibels::decibelsToGain(lowGain_dB)
    );

    *highShelfFilter.state = *FilterCoefs::makeHighShelf(
        currentSampleRate, highFreqHz, Q,
        juce::Decibels::decibelsToGain(highGain_dB)
    );
}

void ToneControl::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!isPrepared)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    lowShelfFilter.process(context);
    highShelfFilter.process(context);
}

void ToneControl::reset()
{
    lowShelfFilter.reset();
    highShelfFilter.reset();
}
