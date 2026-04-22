/*
  ==============================================================================

    MIDI Note Mapping System for Round Robin Lite

    Maps 10 key pairs for footstep sample organization.
    ALL samples play at original pitch (no pitch shifting).
    C1 (MIDI 36) is the root reference note.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 * MidiMapper - Static utility class for Round Robin Lite's key pair system.
 *
 * KEY PAIR CONCEPT:
 * Round Robin Lite uses PAIRS of white keys for organizing samples.
 * This allows users to easily alternate between two notes to create
 * realistic footstep patterns (left foot, right foot).
 *
 * IMPORTANT: Unlike traditional samplers, Round Robin Lite does NOT
 * pitch-shift samples. All samples play at their original pitch regardless
 * of which MIDI key is pressed.
 *
 * MAPPING TABLE (For Sample Organization Only - No Pitch Shifting):
 * Pair Index | Keys     | MIDI Notes | Use Case
 * -----------|----------|------------|---------------------------
 * 0          | C0/D0    | 24/26      | Sample Slot 1 & 2
 * 1          | E0/F0    | 28/29      | Sample Slot 3 & 4
 * 2          | G0/A0    | 31/33      | Sample Slot 5 & 6
 * 3          | B0/C1    | 35/36      | Sample Slot 7 & 8
 * 4          | E1/F1    | 40/41      | Sample Slot 9 & 10
 * 5          | G1/A1    | 43/45      | Sample Slot 11 & 12
 * 6          | B1/C2    | 47/48      | Sample Slot 13 & 14
 * 7          | C2/D2    | 48/50      | Sample Slot 15 & 16
 * 8          | E2/F2    | 52/53      | Sample Slot 17 & 18
 * 9          | G2/A2    | 55/57      | Sample Slot 19 & 20
 *
 * ROOT NOTE: C1 (MIDI 36) is the reference pitch. All samples play at
 * their original, unmodified pitch regardless of which key triggers them.
 */

class MidiMapper
{
public:
    //==============================================================================
    // Constants

    static constexpr int NUM_KEY_PAIRS = 10;        // Total number of key pairs
    static constexpr int ROOT_PAIR_INDEX = 3;       // B0/C1 pair (contains C1)
    static constexpr int ROOT_MIDI_NOTE = 36;       // C1 is the root reference pitch

    //==============================================================================
    /**
     * Get the two MIDI note numbers for a given key pair index.
     *
     * @param pairIndex The key pair index (0-9)
     * @param note1 [out] First MIDI note of the pair
     * @param note2 [out] Second MIDI note of the pair
     * @return true if pairIndex is valid (0-9), false otherwise
     *
     * Example:
     *   int n1, n2;
     *   MidiMapper::getMidiNotesForPair(3, n1, n2);
     *   // Result: n1 = 35 (B0), n2 = 36 (C1/Root)
     */
    static bool getMidiNotesForPair(int pairIndex, int& note1, int& note2)
    {
        if (pairIndex < 0 || pairIndex >= NUM_KEY_PAIRS)
            return false;

        // Define the complete mapping table
        // Each pair: {first note, second note}
        static const int pairMapping[NUM_KEY_PAIRS][2] =
        {
            { 24, 26 },  // Pair 0: C0/D0
            { 28, 29 },  // Pair 1: E0/F0
            { 31, 33 },  // Pair 2: G0/A0
            { 35, 36 },  // Pair 3: B0/C1 (ROOT - C1 reference)
            { 40, 41 },  // Pair 4: E1/F1
            { 43, 45 },  // Pair 5: G1/A1
            { 47, 48 },  // Pair 6: B1/C2
            { 48, 50 },  // Pair 7: C2/D2
            { 52, 53 },  // Pair 8: E2/F2
            { 55, 57 }   // Pair 9: G2/A2
        };

        note1 = pairMapping[pairIndex][0];
        note2 = pairMapping[pairIndex][1];
        return true;
    }

    //==============================================================================
    /**
     * Get the semitone offset for a given key pair index.
     *
     * NOTE: In Round Robin Lite, all samples play UNPITCHED.
     * This function returns 0 for all key pairs as a placeholder.
     * It exists for potential future pitch-shifting features.
     *
     * @param pairIndex The key pair index (0-9)
     * @return Always returns 0 (no pitch shifting)
     */
    static int getSemitoneOffsetForPair(int pairIndex)
    {
        if (pairIndex < 0 || pairIndex >= NUM_KEY_PAIRS)
            return 0;

        // All samples play at original pitch - no offset applied
        return 0;
    }

    //==============================================================================
    /**
     * Check if a key pair index is valid.
     *
     * @param pairIndex The key pair index to validate
     * @return true if pairIndex is within valid range (0-9)
     */
    static bool isValidPairIndex(int pairIndex)
    {
        return pairIndex >= 0 && pairIndex < NUM_KEY_PAIRS;
    }

    //==============================================================================
    /**
     * Get a human-readable name for a key pair.
     *
     * @param pairIndex The key pair index (0-9)
     * @return String like "C0/D0" or "C1/D1" (ROOT)" or "Invalid"
     *
     * Example:
     *   MidiMapper::getKeyPairName(3) returns "B0/C1 (ROOT)"
     */
    static juce::String getKeyPairName(int pairIndex)
    {
        if (!isValidPairIndex(pairIndex))
            return "Invalid";

        static const juce::String pairNames[NUM_KEY_PAIRS] =
        {
            "C0/D0",
            "E0/F0",
            "G0/A0",
            "B0/C1 (ROOT)",  // Contains C1 root reference
            "E1/F1",
            "G1/A1",
            "B1/C2",
            "C2/D2",
            "E2/F2",
            "G2/A2"
        };

        return pairNames[pairIndex];
    }
};
