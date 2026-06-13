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

    // Invalidate gain cache so the first updateFilters after a (re)prepare
    // always rebuilds coefficients — the sample rate may have changed.
    lastLowGain_dB  = std::numeric_limits<float>::quiet_NaN();
    lastHighGain_dB = std::numeric_limits<float>::quiet_NaN();

    isPrepared = true;
    updateFilters(0.0f, 0.0f);
}

void ToneControl::updateFilters(float lowGain_dB, float highGain_dB)
{
    if (!isPrepared)
        return;

    lowGain_dB  = juce::jlimit(-12.0f, 12.0f, lowGain_dB);
    highGain_dB = juce::jlimit(-12.0f, 12.0f, highGain_dB);

    // Coefficient math is the costly part — only run it when gains actually moved.
    constexpr float epsilon = 0.001f;
    if (std::abs(lowGain_dB  - lastLowGain_dB)  < epsilon &&
        std::abs(highGain_dB - lastHighGain_dB) < epsilon)
        return;

    *lowShelfFilter.state = *FilterCoefs::makeLowShelf(
        currentSampleRate, lowFreqHz, Q,
        juce::Decibels::decibelsToGain(lowGain_dB)
    );

    *highShelfFilter.state = *FilterCoefs::makeHighShelf(
        currentSampleRate, highFreqHz, Q,
        juce::Decibels::decibelsToGain(highGain_dB)
    );

    lastLowGain_dB  = lowGain_dB;
    lastHighGain_dB = highGain_dB;
}

void ToneControl::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!isPrepared)
        return;

    // Hosts legally hand empty blocks (transport edges, loop boundaries). A
    // 0-sample dsp::AudioBlock trips getChannelPointer's numSamples>0 assert and
    // does no useful work, so bail early.
    if (buffer.getNumSamples() == 0)
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
