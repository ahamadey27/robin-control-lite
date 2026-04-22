#pragma once
#include <array>

//==============================================================================
// Random Algorithm tick-to-parameter offset table.
// Each tick defines additive offsets applied ON TOP of the user's manual
// randomization settings. Tick 0 = no additional randomization.
//
// Values are stored in the same units as the corresponding APVTS
// randomization parameters (0-1 for float rnd params, raw value for int rnd).

struct RandomAlgorithmOffsets
{
    float volumeRndNeg      = 0.0f;
    float volumeRndPos      = 0.0f;
    
    float semitoneRndNeg    = 0.0f;
    float semitoneRndPos    = 0.0f;
    
    float fineTuneRndNeg    = 0.0f;
    float fineTuneRndPos    = 0.0f;
    
    float toneHighRndNeg    = 0.0f;
    float toneHighRndPos    = 0.0f;
    
    float toneLowRndNeg     = 0.0f;
    float toneLowRndPos     = 0.0f;
    
    float sampleStartRndPos = 0.0f;
    
    float panRndNeg         = 0.0f;
    float panRndPos         = 0.0f;
};

inline constexpr int kNumAlgorithmTicks = 18;

inline const std::array<RandomAlgorithmOffsets, kNumAlgorithmTicks> randomAlgorithmTable = {{
    // Tick 0: off
    {},

    // Tick 1
    { 0.03f, 0.03f,   0.0f, 0.0f,   8.0f, 8.0f,
      0.0f, 0.0f,     0.0f, 0.0f,   0.0f, 0.0f, 0.0f },

    // Tick 2
    { 0.05f, 0.05f,   0.0f, 0.0f,   14.0f, 14.0f,
      0.0f, 0.0f,     0.0f, 0.0f,   0.0f, 0.0f, 0.0f },

    // Tick 3
    { 0.08f, 0.08f,   0.0f, 0.0f,   14.0f, 14.0f,
      0.1f, 0.1f,     0.0f, 0.0f,   0.0f, 0.0f, 0.0f },

    // Tick 4
    { 0.1f, 0.1f,   0.0f, 0.0f,   14.0f, 14.0f,
      0.1f, 0.1f,     0.1f, 0.1f,   0.0f, 0.0f, 0.0f },

    // Tick 5
    { 0.13f, 0.13f,     0.0f, 0.0f,   20.0f, 20.0f,
      0.1f, 0.1f,     0.1f, 0.1f,   0.0f, 0.0f, 0.0f },

    // Tick 6
    { 0.15f, 0.15f,     0.0f, 0.0f,   20.0f, 20.0f,
      0.2f, 0.2f,     0.2f, 0.2f,   0.0f, 0.0f, 0.0f },

    // Tick 7
    { 0.18f, 0.18f,     0.0f, 0.0f,   40.0f, 40.0f,
      0.25f, 0.25f,   0.25f, 0.25f, 0.0f, 0.0f, 0.0f },

    // Tick 8
    { 0.2f, 0.2f,   0.0f, 0.0f,   45.0f, 45.0f,
      0.3f, 0.3f,     0.3f, 0.3f,   0.0f, 0.0f, 0.0f },

    // Tick 9
    { 0.2f, 0.2f,     0.0f, 0.0f,   60.0f, 60.0f,
      0.3f, 0.3f,     0.3f, 0.3f,   0.0f, 0.0f, 0.0f },

    // Tick 10
    { 0.2f, 0.2f,     0.0f, 0.0f,   100.0f, 100.0f,
      0.35f, 0.35f,   0.35f, 0.35f, 0.0f, 0.0f, 0.0f },

    // Tick 11
    { 0.2f, 0.2f,   0.0f, 0.0f,   100.0f, 100.0f,
      0.38f, 0.38f,   0.38f, 0.38f, 0.2f, 0.0f, 0.0f },

    // Tick 12
    { 0.2f, 0.2f,   2.0f, 2.0f,   10.0f, 10.0f,
      0.39f, 0.39f,   0.39f, 0.39f, 0.0f, 0.0f, 0.0f },

    // Tick 13
    { 0.22f, 0.22f,   3.0f, 3.0f,   10.0f, 10.0f,
      0.45f, 0.45f,   0.45f, 0.45f, 0.0f, 0.0f, 0.0f },

    // Tick 14
    { 0.26f, 0.26f,     4.0f, 4.0f,   30.0f, 30.0f,
      0.45f, 0.45f,   0.5f, 0.5f,   0.0f, 0.0f, 0.0f },

    // Tick 15
    { 0.2f, 0.3f,     4.0f, 4.0f,   40.0f, 40.0f,
      0.4f, 0.4f,     0.5f, 0.5f,   0.12f, 0.0f, 0.0f },

    // Tick 16
    { 0.2f, 0.31f,     4.0f, 5.0f,   50.0f, 45.0f,
      0.5f, 0.5f,     0.5f, 0.5f,   0.12f, 0.0f, 0.0f },

    // Tick 17
    { 0.2f, 0.32f,   5.0f, 6.5f,   50.0f, 50.0f,
      0.55f, 0.55f,   0.55f, 0.55f, 0.12f, 0.15f, 0.15f },
}};
