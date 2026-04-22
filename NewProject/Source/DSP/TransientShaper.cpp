/*
  ==============================================================================

    TransientShaper.cpp
    Implementation of Transient Master

  ==============================================================================
*/

#include "TransientShaper.h"

TransientShaper::TransientShaper()
{
    // Constructor - time constants set in prepareToPlay()
}

void TransientShaper::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Fast envelope (transients): ~5ms attack, ~50ms release
    fastAttack = std::exp(-1.0f / (0.005f * static_cast<float>(sampleRate)));
    fastRelease = std::exp(-1.0f / (0.050f * static_cast<float>(sampleRate)));

    // Slow envelope (sustain): ~50ms attack, ~500ms release
    slowAttack = std::exp(-1.0f / (0.050f * static_cast<float>(sampleRate)));
    slowRelease = std::exp(-1.0f / (0.500f * static_cast<float>(sampleRate)));

    // Reset envelope states
    reset();

    isPrepared = true;

    DBG("TransientShaper prepared - Sample Rate: " + juce::String(sampleRate) + " Hz");
}

void TransientShaper::processBlock(juce::AudioBuffer<float>& buffer, float attackAmount, float decayAmount)
{
    if (!isPrepared)
    {
        DBG("WARNING: TransientShaper::processBlock() called before prepareToPlay()");
        return;
    }

    // If both parameters are at 0, bypass processing
    if (attackAmount == 0.0f && decayAmount == 0.0f)
        return;

    // Convert parameter values to gain multipliers
    float attackGain = parameterToGain(attackAmount);
    float decayGain = parameterToGain(decayAmount);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        // Select envelope states for this channel
        float& fastEnv = (channel == 0) ? fastEnvelopeL : fastEnvelopeR;
        float& slowEnv = (channel == 0) ? slowEnvelopeL : slowEnvelopeR;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];

            // Detect fast and slow envelopes
            float fastLevel = detectEnvelope(input, fastEnv, true);
            float slowLevel = detectEnvelope(input, slowEnv, false);

            // Separate transient and sustain components
            // Transient = difference between fast and slow envelopes
            float transientLevel = juce::jmax(0.0f, fastLevel - slowLevel);
            float sustainLevel = slowLevel;

            // Calculate mix based on envelope difference
            // If transient is strong, apply attack gain; otherwise apply decay gain
            float totalLevel = fastLevel + 0.0001f; // Avoid division by zero
            float transientRatio = transientLevel / totalLevel;

            // Blend between attack and decay gains
            float finalGain = transientRatio * attackGain + (1.0f - transientRatio) * decayGain;

            // Apply gain to sample
            channelData[sample] = input * finalGain;
        }
    }
}

void TransientShaper::reset()
{
    fastEnvelopeL = 0.0f;
    fastEnvelopeR = 0.0f;
    slowEnvelopeL = 0.0f;
    slowEnvelopeR = 0.0f;

    DBG("TransientShaper reset");
}

float TransientShaper::parameterToGain(float paramValue)
{
    // Map -127 to +127 → 0.5x to 2.0x
    // -127 = 0.5x (softer)
    // 0 = 1.0x (no change)
    // +127 = 2.0x (louder)

    float normalized = paramValue / 127.0f; // -1.0 to +1.0

    if (normalized < 0.0f)
    {
        // Negative: interpolate from 0.5x to 1.0x
        return 0.5f + (normalized + 1.0f) * 0.5f;
    }
    else
    {
        // Positive: interpolate from 1.0x to 2.0x
        return 1.0f + normalized;
    }
}

float TransientShaper::detectEnvelope(float input, float& envelopeState, bool isFast)
{
    // Get absolute value of input
    float rectified = std::abs(input);

    // Select attack/release coefficients
    float attack = isFast ? fastAttack : slowAttack;
    float release = isFast ? fastRelease : slowRelease;

    // Envelope follower with separate attack/release
    if (rectified > envelopeState)
    {
        // Attack (rising)
        envelopeState = attack * envelopeState + (1.0f - attack) * rectified;
    }
    else
    {
        // Release (falling)
        envelopeState = release * envelopeState + (1.0f - release) * rectified;
    }

    return envelopeState;
}
