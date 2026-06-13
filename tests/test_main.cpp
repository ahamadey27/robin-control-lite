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
#include <thread>
#include <atomic>
#include <chrono>

#include "RandomizationEngine.h"
#include "MidiMapper.h"
#include "PluginProcessor.h"

//==============================================================================
// Shared helpers for the processor-level harnesses below.
namespace rcltest
{
    // Writes `lengthSamples` of deterministic pseudo-noise as a 16-bit mono WAV to
    // a temp file and returns it. The caller owns the file (call deleteFile()).
    // A real loaded sample makes processBlock actually render — exercising the
    // RRVoice playback / start-end clamp / tone path, not just an idle engine.
    inline juce::File writeTempWav (juce::Random& rng, int lengthSamples, double sr = 44100.0)
    {
        auto file = juce::File::createTempFile (".wav");
        juce::WavAudioFormat fmt;

        if (auto os = file.createOutputStream())
        {
            std::unique_ptr<juce::OutputStream> stream (std::move (os));
            const auto options = juce::AudioFormatWriterOptions{}
                                     .withSampleRate (sr)
                                     .withNumChannels (1)
                                     .withBitsPerSample (16);
            auto writer = fmt.createWriterFor (stream, options);

            if (writer != nullptr)
            {
                juce::AudioBuffer<float> buf (1, lengthSamples);
                for (int i = 0; i < lengthSamples; ++i)
                    buf.setSample (0, i, rng.nextFloat() * 2.0f - 1.0f);
                writer->writeFromAudioSampleBuffer (buf, 0, lengthSamples);
            }
        }
        return file;
    }

    // Loads `wav` into the first `count` slots and wires the synth, mirroring the
    // lock discipline the processor uses in setStateInformation's restore block.
    inline void loadIntoSlots (NewProjectAudioProcessor& proc, const juce::File& wav, int count)
    {
        for (int i = 0; i < count; ++i)
            proc.sampleLoader.loadSample (i, wav);
        {
            const juce::ScopedLock sl (proc.getCallbackLock());
            proc.rebuildLoadedIndices();
            proc.reshuffleIndices();
        }
        proc.sampleLoader.updateSynthesiserSounds();
    }
}

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
// Parameter / automation fuzz. Drives every APVTS parameter across its range
// (including the 0.0 / 1.0 extremes) interleaved with rendering of a real loaded
// sample — the "automation hammering" pluginval applies at strictness 8–10, but
// in-process so sanitizers see it. Catches UB in smoothing / voice-param updates
// and any listener (parameterChanged) mishandling.
class ProcessorAutomationTests : public juce::UnitTest
{
public:
    ProcessorAutomationTests() : juce::UnitTest ("ProcessorAutomation") {}

    void runTest() override
    {
        juce::Random rng (0xA0B1C2);
        beginTest ("sweep all parameters while rendering a loaded sample");
        {
            auto wav = rcltest::writeTempWav (rng, 22050);   // ~0.5 s @ 44.1k
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            rcltest::loadIntoSlots (*proc, wav, 3);
            proc->prepareToPlay (48000.0, 256);

            auto& params = proc->getParameters();
            expect (params.size() > 0, "processor exposes no parameters");

            // Pin every parameter to both extremes once — the values most likely
            // to expose a clamp/divide/denormal bug.
            for (auto* p : params) { p->setValueNotifyingHost (0.0f); p->setValueNotifyingHost (1.0f); }

            juce::AudioBuffer<float> buffer (2, 256);
            for (int iter = 0; iter < 4000; ++iter)
            {
                // A few random automation moves per block.
                const int moves = rng.nextInt ({ 1, 6 });
                for (int m = 0; m < moves; ++m)
                    if (auto* p = params[rng.nextInt (params.size())])
                        p->setValueNotifyingHost (rng.nextFloat());

                buffer.clear();
                juce::MidiBuffer midi;
                if (rng.nextInt (4) == 0)
                    midi.addEvent (juce::MidiMessage::noteOn (1, rng.nextInt ({ 24, 96 }),
                                                             (juce::uint8) rng.nextInt ({ 1, 128 })), 0);
                proc->processBlock (buffer, midi);

                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                    for (int s = 0; s < buffer.getNumSamples(); ++s)
                        expect (std::isfinite (buffer.getSample (ch, s)), "non-finite under automation");
            }

            proc->releaseResources();
            wav.deleteFile();
        }
    }
};

//==============================================================================
// Lifecycle / repeated load-unload. Constructs, prepares, loads samples,
// renders, save/restores, releases, and destroys the whole processor many times.
// This is the classic crash class pluginval's repeated open/close hits — a
// dangling parameter listener (missing removeListener in the dtor), a static
// left in a bad state, or a leak the JUCE leak-detector will assert on.
class ProcessorLifecycleTests : public juce::UnitTest
{
public:
    ProcessorLifecycleTests() : juce::UnitTest ("ProcessorLifecycle") {}

    void runTest() override
    {
        juce::Random rng (0xD3E4F5);
        beginTest ("repeated construct/prepare/load/process/save-restore/destroy cycles");
        {
            auto wav = rcltest::writeTempWav (rng, 8000);
            const double srs[] { 44100.0, 48000.0, 96000.0 };

            for (int cycle = 0; cycle < 40; ++cycle)
            {
                auto proc = std::make_unique<NewProjectAudioProcessor>();
                const double sr = srs[cycle % 3];
                const int    bs = 1 << (6 + (cycle % 4));   // 64..512

                // ~1 in 5 cycles: construct and destroy without ever preparing or
                // processing — the bare open/close a host does when scanning. The
                // rest run the full prepare→process path (a host never calls
                // processBlock before prepareToPlay, so we never do either).
                if (rng.nextInt (5) == 0)
                {
                    if (rng.nextBool())
                        rcltest::loadIntoSlots (*proc, wav, rng.nextInt ({ 1, 4 }));
                    continue;   // proc destructs here — dtor must unwind cleanly.
                }

                proc->prepareToPlay (sr, bs);

                if (rng.nextBool())
                    rcltest::loadIntoSlots (*proc, wav, rng.nextInt ({ 1, 4 }));

                juce::AudioBuffer<float> buffer (2, bs);
                for (int b = 0; b < 6; ++b)
                {
                    buffer.clear();
                    juce::MidiBuffer midi;
                    if (b == 0) midi.addEvent (juce::MidiMessage::noteOn (1, 48, 0.9f), 0);
                    proc->processBlock (buffer, midi);
                }

                juce::MemoryBlock state;
                proc->getStateInformation (state);
                proc->setStateInformation (state.getData(), (int) state.getSize());

                if (rng.nextBool()) proc->releaseResources();   // vary teardown order
                // proc destructs here — must remove its parameter listeners cleanly.
            }

            wav.deleteFile();
            expect (true, "survived repeated lifecycle cycles");
        }
    }
};

//==============================================================================
// Thread-race harness — the TSan target TESTING.md §3 calls out explicitly:
// setStateInformation / pool edits / automation on the message thread racing
// processBlock on the audio thread. We emulate the host faithfully: the audio
// thread holds getCallbackLock() around processBlock (JUCE format wrappers do
// this), which is the lock the message-thread mutators take internally. A clean
// TSan run proves nothing shared escapes that lock.
class ProcessorConcurrencyTests : public juce::UnitTest
{
public:
    ProcessorConcurrencyTests() : juce::UnitTest ("ProcessorConcurrency") {}

    void runTest() override
    {
        juce::Random rng (0x1A2B3C);
        beginTest ("concurrent state I/O + automation vs processBlock");
        {
            auto wav = rcltest::writeTempWav (rng, 16000);
            auto proc = std::make_unique<NewProjectAudioProcessor>();
            rcltest::loadIntoSlots (*proc, wav, 4);
            proc->prepareToPlay (48000.0, 256);   // fixed block size for the whole race

            // A valid saved state (sample present) for the restore storm.
            juce::MemoryBlock saved;
            proc->getStateInformation (saved);

            std::atomic<bool> stop { false };
            std::atomic<int>  audioBlocks { 0 };

            // Audio thread: render under the callback lock, as a host wrapper does.
            std::thread audio ([&]
            {
                juce::Random arng (0x5EED5);
                juce::AudioBuffer<float> buf (2, 256);
                while (! stop.load (std::memory_order_relaxed))
                {
                    buf.clear();
                    juce::MidiBuffer midi;
                    if (arng.nextInt (3) == 0)
                        midi.addEvent (juce::MidiMessage::noteOn (1, arng.nextInt ({ 24, 96 }), 0.8f), 0);
                    {
                        const juce::ScopedLock sl (proc->getCallbackLock());
                        proc->processBlock (buf, midi);
                    }
                    audioBlocks.fetch_add (1, std::memory_order_relaxed);
                    // Emulate the inter-buffer gap a real host leaves (256/48k ≈
                    // 5 ms). Without this the lock-holding loop starves the message
                    // thread — a test artifact, not a plugin defect.
                    std::this_thread::sleep_for (std::chrono::microseconds (300));
                }
            });

            // Message thread (here): hammer everything a host/UI can do live.
            auto& params = proc->getParameters();
            for (int i = 0; i < 600; ++i)
            {
                if (auto* p = params[rng.nextInt (params.size())])
                    p->setValueNotifyingHost (rng.nextFloat());

                switch (rng.nextInt (8))
                {
                    case 0: proc->setStateInformation (saved.getData(), (int) saved.getSize()); break;
                    case 1: proc->swapSamples   (rng.nextInt (4), rng.nextInt (4)); break;
                    case 2: proc->insertSample  (rng.nextInt (4), rng.nextInt (4)); break;
                    case 3: proc->auditionSample (rng.nextInt (4)); break;
                    case 4: proc->resetPlaybackPosition(); break;
                    case 5: proc->requestTrigger(); break;
                    case 6: proc->requestPanic(); break;
                    default: break;
                }
            }

            stop.store (true, std::memory_order_relaxed);
            audio.join();

            proc->releaseResources();
            wav.deleteFile();
            expect (audioBlocks.load() > 0, "audio thread never ran a block");
        }
    }
};

//==============================================================================
// Auto-registering instances.
static RandomizationEngineTests randomizationEngineTests;
static MidiMapperTests          midiMapperTests;
static ProcessorFuzzTests        processorFuzzTests;
static ProcessorAutomationTests  processorAutomationTests;
static ProcessorLifecycleTests   processorLifecycleTests;
static ProcessorConcurrencyTests processorConcurrencyTests;

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
