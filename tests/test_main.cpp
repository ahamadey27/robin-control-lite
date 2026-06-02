/*
  ==============================================================================
    Unit tests for Robin Control Lite — pure-logic layer.

    Uses JUCE's built-in UnitTest framework (no extra dependencies). This is the
    "correctness" layer of the testing method described in TESTING.md: it catches
    logic regressions that host validators (pluginval/auval) cannot see.

    The target is a plain console app, so the WHOLE process is instrumented when
    built with a sanitizer — making this the clean vehicle for ASan/TSan/UBSan
    (see scripts/test-sanitizers.sh).

    Add new test classes here as the engine grows. Each `static` instance below
    auto-registers with the runner.
  ==============================================================================
*/

#include <JuceHeader.h>
#include <iostream>

#include "RandomizationEngine.h"
#include "MidiMapper.h"

//==============================================================================
class RandomizationEngineTests : public juce::UnitTest
{
public:
    RandomizationEngineTests() : juce::UnitTest ("RandomizationEngine") {}

    void runTest() override
    {
        beginTest ("zero ranges return the base value unchanged");
        {
            RandomizationEngine e;
            for (int i = 0; i < 256; ++i)
                expectEquals (e.generateRandomValue (5.0f, 0.0f, 0.0f), 5.0f);
        }

        beginTest ("asymmetric range stays within [base-neg, base+pos)");
        {
            RandomizationEngine e;
            const float base = 10.0f, neg = 3.0f, pos = 7.0f;
            for (int i = 0; i < 100000; ++i)
            {
                const float v = e.generateRandomValue (base, neg, pos);
                expect (v >= base - neg, "below lower bound: " + juce::String (v));
                expect (v <  base + pos, "at/above upper bound: " + juce::String (v));
            }
        }

        beginTest ("only-positive range never dips below base");
        {
            RandomizationEngine e;
            for (int i = 0; i < 20000; ++i)
            {
                const float v = e.generateRandomValue (0.0f, 0.0f, 5.0f);
                expect (v >= 0.0f && v < 5.0f, "out of range: " + juce::String (v));
            }
        }

        beginTest ("only-negative range never rises above base");
        {
            RandomizationEngine e;
            for (int i = 0; i < 20000; ++i)
            {
                const float v = e.generateRandomValue (0.0f, 5.0f, 0.0f);
                expect (v >= -5.0f && v < 0.0f, "out of range: " + juce::String (v));
            }
        }

        beginTest ("same seed produces an identical sequence (deterministic)");
        {
            RandomizationEngine a, b;
            a.resetSeed (12345);
            b.resetSeed (12345);
            for (int i = 0; i < 1000; ++i)
                expectEquals (a.generateRandomValue (0.0f, 1.0f, 1.0f),
                              b.generateRandomValue (0.0f, 1.0f, 1.0f));
        }
    }
};

//==============================================================================
class MidiMapperTests : public juce::UnitTest
{
public:
    MidiMapperTests() : juce::UnitTest ("MidiMapper") {}

    void runTest() override
    {
        beginTest ("valid pair indices return the documented notes");
        {
            int n1 = 0, n2 = 0;
            expect (MidiMapper::getMidiNotesForPair (0, n1, n2));
            expectEquals (n1, 24); expectEquals (n2, 26);

            expect (MidiMapper::getMidiNotesForPair (3, n1, n2));   // ROOT pair
            expectEquals (n1, 35); expectEquals (n2, 36);           // 36 == C1 root

            expect (MidiMapper::getMidiNotesForPair (9, n1, n2));
            expectEquals (n1, 55); expectEquals (n2, 57);
        }

        beginTest ("out-of-range indices are rejected and outputs left untouched");
        {
            int n1 = -1, n2 = -1;
            expect (! MidiMapper::getMidiNotesForPair (-1, n1, n2));
            expect (! MidiMapper::getMidiNotesForPair (MidiMapper::NUM_KEY_PAIRS, n1, n2));
            expectEquals (n1, -1); expectEquals (n2, -1);
        }

        beginTest ("ROOT constants are self-consistent");
        {
            int n1 = 0, n2 = 0;
            MidiMapper::getMidiNotesForPair (MidiMapper::ROOT_PAIR_INDEX, n1, n2);
            expect (n1 == MidiMapper::ROOT_MIDI_NOTE || n2 == MidiMapper::ROOT_MIDI_NOTE,
                    "ROOT_PAIR_INDEX should contain ROOT_MIDI_NOTE (36)");
        }

        beginTest ("isValidPairIndex matches the documented range");
        {
            expect (! MidiMapper::isValidPairIndex (-1));
            expect (  MidiMapper::isValidPairIndex (0));
            expect (  MidiMapper::isValidPairIndex (9));
            expect (! MidiMapper::isValidPairIndex (10));
        }

        beginTest ("names: invalid -> \"Invalid\", root -> contains \"ROOT\"");
        {
            expectEquals (MidiMapper::getKeyPairName (-1), juce::String ("Invalid"));
            expect (MidiMapper::getKeyPairName (3).contains ("ROOT"));
        }

        beginTest ("semitone offset is always 0 (unpitched product behaviour)");
        {
            for (int i = -2; i < MidiMapper::NUM_KEY_PAIRS + 2; ++i)
                expectEquals (MidiMapper::getSemitoneOffsetForPair (i), 0);
        }
    }
};

//==============================================================================
// Auto-registering instances.
static RandomizationEngineTests randomizationEngineTests;
static MidiMapperTests          midiMapperTests;

int main (int, char**)
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        if (auto* r = runner.getResult (i))
            failures += r->failures;

    if (failures > 0)
    {
        std::cerr << "\n❌ " << failures << " unit-test failure(s).\n";
        return 1;
    }

    std::cout << "\n✅ All unit tests passed.\n";
    return 0;
}
