/*
  ==============================================================================

    ThreeBandEQ.cpp
    Implementation of 3-Band Equalizer

  ==============================================================================
*/

#include "ThreeBandEQ.h"

ThreeBandEQ::ThreeBandEQ()
{
    // Constructor - filters will be initialized in prepareToPlay()
}

void ThreeBandEQ::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Set up processing spec for all filters
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2; // Stereo

    // Initialize all three filters
    lowShelfFilter.prepare(spec);
    midPeakFilter.prepare(spec);
    highShelfFilter.prepare(spec);

    // Initialize with neutral settings (0dB gain)
    updateFilters(0.0f, 100.0f,   // Low: 0dB @ 100Hz
        0.0f, 1000.0f,  // Mid: 0dB @ 1000Hz
        0.0f, 5000.0f); // High: 0dB @ 5000Hz

    isPrepared = true;

    DBG("ThreeBandEQ prepared - Sample Rate: " + juce::String(sampleRate) +
        " Hz, Buffer Size: " + juce::String(samplesPerBlock));
}

void ThreeBandEQ::updateFilters(float lowGain_dB, float lowFreq_Hz,
    float midGain_dB, float midFreq_Hz,
    float highGain_dB, float highFreq_Hz)
{
    if (!isPrepared)
    {
        DBG("WARNING: ThreeBandEQ::updateFilters() called before prepareToPlay()");
        return;
    }

    // Clamp frequencies to safe ranges to prevent instability
    lowFreq_Hz = juce::jlimit(20.0f, 500.0f, lowFreq_Hz);
    midFreq_Hz = juce::jlimit(200.0f, 5000.0f, midFreq_Hz);
    highFreq_Hz = juce::jlimit(2000.0f, 20000.0f, highFreq_Hz);

    // Clamp gains to prevent extreme values
    lowGain_dB = juce::jlimit(-24.0f, 24.0f, lowGain_dB);
    midGain_dB = juce::jlimit(-24.0f, 24.0f, midGain_dB);
    highGain_dB = juce::jlimit(-24.0f, 24.0f, highGain_dB);

    // Q values — lower = gentler/wider, higher = sharper/narrower
    constexpr float shelfQ = 0.5f;   // softer shelf slopes for low and high
    constexpr float peakQ = 0.6f;   // broader bell for mid, less ringy

    // LOW SHELF FILTER
    // Boosts/cuts frequencies below lowFreq_Hz
    *lowShelfFilter.state = *FilterCoefs::makeLowShelf(
        currentSampleRate, lowFreq_Hz, shelfQ,     // was Q
        juce::Decibels::decibelsToGain(lowGain_dB)
    );

    // MID PEAK FILTER (Bell/Parametric)
    // Boosts/cuts frequencies around midFreq_Hz
    *midPeakFilter.state = *FilterCoefs::makePeakFilter(
        currentSampleRate, midFreq_Hz, peakQ,      // was Q
        juce::Decibels::decibelsToGain(midGain_dB)
    );

    // HIGH SHELF FILTER
    // Boosts/cuts frequencies above highFreq_Hz
    *highShelfFilter.state = *FilterCoefs::makeHighShelf(
        currentSampleRate, highFreq_Hz, shelfQ,    // was Q
        juce::Decibels::decibelsToGain(highGain_dB)
    );
}

void ThreeBandEQ::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!isPrepared)
    {
        DBG("WARNING: ThreeBandEQ::processBlock() called before prepareToPlay()");
        return;
    }

    // Create audio block for JUCE DSP processing
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process filters in series: Low → Mid → High
    lowShelfFilter.process(context);
    midPeakFilter.process(context);
    highShelfFilter.process(context);
}

void ThreeBandEQ::reset()
{
    lowShelfFilter.reset();
    midPeakFilter.reset();
    highShelfFilter.reset();

    DBG("ThreeBandEQ filters reset");
}
