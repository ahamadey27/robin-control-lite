#pragma once
#include <JuceHeader.h>

class RandomizationEngine
{
public:
    RandomizationEngine();

    // Generate single randomized value
    float generateRandomValue(float baseValue, float negRange, float posRange);

    // Reset random seed (for testing or user control)
    void resetSeed(juce::int64 newSeed = juce::Time::currentTimeMillis());

private:
    juce::Random random;
};
