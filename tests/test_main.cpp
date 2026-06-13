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
#include "PluginProcessor.h"

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
// Processor crash-class harness (TESTING.md §3 "next increment"). Instantiates
// the REAL NewProjectAudioProcessor headlessly and hammers the two paths that
// produce nearly all field crashes — processBlock and setStateInformation — with
// adversarial inputs: sample-rate flips, varied/oversized blocks, MIDI floods,
// trigger/panic races, and malformed/truncated saved state.
//
// This test asserts nothing about audio CONTENT; its job is to give the
// sanitizers (ASan/UBSan/TSan, via scripts/test-sanitizers.sh) instrumented
// reach into these paths. A clean run = no OOB/UAF/UB/race on the crash paths.
// The few expect()s here are sanity rails (finite output, no NaN/Inf).
//
// Determinism: a fixed-seed juce::Random drives every choice so a sanitizer hit
// is reproducible.
class ProcessorFuzzTests : public juce::UnitTest
{
public:
    ProcessorFuzzTests() : juce::UnitTest ("ProcessorFuzz") {}

    // True if every sample across every channel is finite (no NaN/Inf escaped).
    static bool isFinite (const juce::AudioBuffer<float>& b)
    {
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            const float* p = b.getReadPointer (ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (! std::isfinite (p[i]))
                    return false;
        }
        return true;
    }

    // Stuff a block's worth of plausible-but-chaotic (but well-formed) MIDI into a
    // buffer. We only emit valid messages: a real host parses raw bytes before
    // processBlock ever sees them, so feeding malformed bytes would only fuzz
    // JUCE's parser, not our engine.
    static void fillRandomMidi (juce::MidiBuffer& m, juce::Random& rng, int numSamples)
    {
        const int events = rng.nextInt ({ 0, 24 });
        for (int e = 0; e < events; ++e)
        {
            const int when = numSamples > 0 ? rng.nextInt (numSamples) : 0;
            switch (rng.nextInt (5))
            {
                case 0: m.addEvent (juce::MidiMessage::noteOn  (1, rng.nextInt ({ 0, 128 }),
                                                                (juce::uint8) rng.nextInt ({ 1, 128 })), when); break;
                case 1: m.addEvent (juce::MidiMessage::noteOff (1, rng.nextInt ({ 0, 128 })), when); break;
                case 2: m.addEvent (juce::MidiMessage::allNotesOff (1), when); break;
                case 3: m.addEvent (juce::MidiMessage::controllerEvent (1, rng.nextInt ({ 0, 128 }),
                                                                        rng.nextInt ({ 0, 128 })), when); break;
                default: m.addEvent (juce::MidiMessage::pitchWheel (1, rng.nextInt ({ 0, 16384 })), when); break;
            }
        }
    }

    // Run one prepared block, exercising trigger/panic flags too. The buffer is
    // always sized to the processor's actual channel count — that's the host
    // contract; handing fewer channels than the active bus would be an invalid
    // scenario no real host produces.
    void pumpBlock (NewProjectAudioProcessor& proc, juce::Random& rng, int numSamples)
    {
        const int numChannels = juce::jmax (1, proc.getTotalNumInputChannels(),
                                               proc.getTotalNumOutputChannels());
        juce::AudioBuffer<float> buffer (numChannels, juce::jmax (0, numSamples));
        buffer.clear();
        juce::MidiBuffer midi;
        fillRandomMidi (midi, rng, buffer.getNumSamples());

        if (rng.nextBool())       proc.requestTrigger();
        if (rng.nextInt (8) == 0) proc.requestPanic();   // panic must win over a same-block trigger

        proc.processBlock (buffer, midi);
        expect (isFinite (buffer), "processBlock produced non-finite output");
    }

    void runTest() override
    {
        juce::Random rng (0xC0FFEE);

        beginTest ("prepare/process across sample-rate + block-size flips");
        {
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            const double sampleRates[] { 44100.0, 48000.0, 88200.0, 96000.0, 22050.0, 192000.0 };
            const int    blockSizes[]  { 1, 16, 64, 128, 512, 4096 };

            for (double sr : sampleRates)
                for (int bs : blockSizes)
                {
                    proc->prepareToPlay (sr, bs);
                    for (int iter = 0; iter < 8; ++iter)
                        pumpBlock (*proc, rng, rng.nextInt ({ 1, bs + 1 }));
                }
            proc->releaseResources();
            expect (true, "survived sample-rate / block-size sweep");
        }

        beginTest ("zero-sample and single-sample blocks");
        {
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            proc->prepareToPlay (48000.0, 512);
            pumpBlock (*proc, rng, 0);   // empty block — common at transport edges
            pumpBlock (*proc, rng, 1);
            for (int i = 0; i < 32; ++i)
                pumpBlock (*proc, rng, rng.nextInt ({ 0, 3 }));
            proc->releaseResources();
        }

        beginTest ("oversized block (more samples than prepared) does not corrupt memory");
        {
            // A misbehaving/automating host can hand a buffer larger than the
            // prepared block. Surface any scratch-buffer OOB to the sanitizer.
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            proc->prepareToPlay (48000.0, 64);
            for (int i = 0; i < 16; ++i)
                pumpBlock (*proc, rng, rng.nextInt ({ 65, 8192 }));
            proc->releaseResources();
        }

        beginTest ("mono output bus");
        {
            // Switch the output bus to mono the way a host would, then process —
            // pumpBlock allocates the now-1-channel buffer to match.
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            auto layout = proc->getBusesLayout();
            if (! layout.outputBuses.isEmpty())
                layout.outputBuses.getReference (0) = juce::AudioChannelSet::mono();
            const bool monoOk = proc->setBusesLayout (layout);
            expect (monoOk, "processor should accept a mono output bus");

            proc->prepareToPlay (44100.0, 256);
            for (int i = 0; i < 16; ++i)
                pumpBlock (*proc, rng, rng.nextInt ({ 1, 257 }));
            proc->releaseResources();
        }

        beginTest ("MIDI flood across many blocks");
        {
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            proc->prepareToPlay (48000.0, 128);
            for (int i = 0; i < 256; ++i)
                pumpBlock (*proc, rng, 128);
            proc->releaseResources();
        }

        beginTest ("setStateInformation tolerates malformed / random / truncated data");
        {
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            proc->prepareToPlay (48000.0, 512);

            // Empty / null inputs.
            proc->setStateInformation (nullptr, 0);
            const char zero = 0;
            proc->setStateInformation (&zero, 0);

            // Pure random byte soup of varying length.
            for (int t = 0; t < 300; ++t)
            {
                const int len = rng.nextInt ({ 0, 4096 });
                juce::MemoryBlock mb ((size_t) len);
                for (int i = 0; i < len; ++i)
                    mb[i] = (char) rng.nextInt (256);
                proc->setStateInformation (mb.getData(), (int) mb.getSize());
                pumpBlock (*proc, rng, 64);   // ensure no latent corruption survives into audio
            }

            // Truncations of a REAL saved state — the nastiest malformed inputs
            // because they parse partway before failing.
            juce::MemoryBlock good;
            proc->getStateInformation (good);
            expect (good.getSize() > 0, "getStateInformation produced empty state");
            const int step = juce::jmax (1, (int) good.getSize() / 17);
            for (int cut = 0; cut <= (int) good.getSize(); cut += step)
            {
                proc->setStateInformation (good.getData(), cut);
                pumpBlock (*proc, rng, 64);
            }

            proc->releaseResources();
        }

        beginTest ("getState/setState round-trip is stable and keeps audio finite");
        {
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            proc->prepareToPlay (48000.0, 512);

            // Nudge a few parameters off their defaults.
            if (auto* p = proc->apvts.getParameter (ParameterIDs::volume))   p->setValueNotifyingHost (0.3f);
            if (auto* p = proc->apvts.getParameter (ParameterIDs::semitone)) p->setValueNotifyingHost (0.8f);
            if (auto* p = proc->apvts.getParameter (ParameterIDs::toneLow))  p->setValueNotifyingHost (0.9f);

            juce::MemoryBlock saved;
            proc->getStateInformation (saved);

            // Round-trip several times — each restore must leave a processable engine.
            for (int i = 0; i < 4; ++i)
            {
                proc->setStateInformation (saved.getData(), (int) saved.getSize());
                for (int b = 0; b < 4; ++b)
                    pumpBlock (*proc, rng, 256);
            }

            juce::MemoryBlock resaved;
            proc->getStateInformation (resaved);
            expect (resaved.getSize() > 0, "re-saved state is empty");
            proc->releaseResources();
        }
    }
};

//==============================================================================
// Auto-registering instances.
static RandomizationEngineTests randomizationEngineTests;
static MidiMapperTests          midiMapperTests;
static ProcessorFuzzTests        processorFuzzTests;

int main (int, char**)
{
    // The processor pulls in APVTS / Synthesiser / message-thread machinery, so a
    // MessageManager must exist for the lifetime of the run. Harmless for the
    // pure-logic tests; required for the processor harness.
    juce::ScopedJuceInitialiser_GUI juceInit;

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
