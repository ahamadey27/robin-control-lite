#include "RandomizationEngine.h"

RandomizationEngine::RandomizationEngine()
{
    random.setSeedRandomly();
}

float RandomizationEngine::generateRandomValue(float baseValue, float negRange, float posRange)
{
    // No randomization if both ranges are 0
    if (negRange == 0.0f && posRange == 0.0f)
        return baseValue;

    // Calculate total range and generate random offset
    float totalRange = negRange + posRange;
    float randomOffset = random.nextFloat() * totalRange - negRange;

    return baseValue + randomOffset;
}

void RandomizationEngine::resetSeed(juce::int64 newSeed)
{
    random.setSeed(newSeed);
}