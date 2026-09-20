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
            // Use the classic createWriterFor overload: it's the API present in the
            // pinned CI JUCE (8.0.4). The newer AudioFormatWriterOptions form isn't
            // in 8.0.4. It's [[deprecated]] in newer JUCE but still compiles (no
            // -Werror). Takes ownership of the stream and deletes it on failure.
            std::unique_ptr<juce::AudioFormatWriter> writer (
                fmt.createWriterFor (os.release(), sr, 1, 16, {}, 0));

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

    // Writes a real audio file in `fmt` with the given shape. Returns an empty
    // (zero-byte) file if the format can't honour the combo (e.g. OGG + 24-bit) —
    // callers should check getSize() and skip. `ext` includes the leading dot.
    inline juce::File writeAudioFile (juce::Random& rng, juce::AudioFormat& fmt,
                                      const juce::String& ext, int numChannels,
                                      int bits, double sr, int lengthSamples)
    {
        auto file = juce::File::createTempFile (ext);
        if (auto os = file.createOutputStream())
        {
            // Classic createWriterFor overload — present in the pinned CI JUCE 8.0.4
            // (the AudioFormatWriterOptions form is newer). Takes ownership of the
            // stream and deletes it (closing the file handle) on failure, so an
            // unsupported combo leaves an empty file the caller skips on getSize()==0.
            std::unique_ptr<juce::AudioFormatWriter> writer (
                fmt.createWriterFor (os.release(), sr, (unsigned int) numChannels, bits, {}, 0));

            if (writer != nullptr)
            {
                juce::AudioBuffer<float> buf (numChannels, lengthSamples);
                for (int ch = 0; ch < numChannels; ++ch)
                    for (int i = 0; i < lengthSamples; ++i)
                        buf.setSample (ch, i, rng.nextFloat() * 2.0f - 1.0f);
                writer->writeFromAudioSampleBuffer (buf, 0, lengthSamples);
            }
        }
        return file;
    }

    // Writes `len` random bytes to a temp file with extension `ext` — a "valid
    // extension, garbage contents" file that must be rejected, not crash.
    inline juce::File writeGarbage (juce::Random& rng, const juce::String& ext, int len)
    {
        auto file = juce::File::createTempFile (ext);
        juce::MemoryBlock mb ((size_t) juce::jmax (0, len));
        for (int i = 0; i < len; ++i)
            mb[i] = (char) rng.nextInt (256);
        file.replaceWithData (mb.getData(), mb.getSize());
        return file;
    }

    // Hand-builds a canonical 16-bit-mono WAV whose `data` chunk size CLAIMS
    // `claimedFrames` frames while only `actualDataBytes` of real samples follow.
    // Used to exercise SampleSlot::loadFromFile's decoded-length guard without
    // allocating (or OOM-ing on) the claimed size.
    inline juce::File writeWavWithClaimedFrames (juce::int64 claimedFrames,
                                                 int actualDataBytes, double sr = 44100.0)
    {
        const int blockAlign = 2;   // 1 channel * 16-bit
        const juce::int64 claimedData = claimedFrames * blockAlign;

        juce::MemoryOutputStream mo;
        mo.write ("RIFF", 4);
        mo.writeInt ((int) (36 + claimedData));      // RIFF size (claimed)
        mo.write ("WAVE", 4);
        mo.write ("fmt ", 4);
        mo.writeInt (16);
        mo.writeShort (1);                            // PCM
        mo.writeShort (1);                            // mono
        mo.writeInt ((int) sr);
        mo.writeInt ((int) sr * blockAlign);          // byte rate
        mo.writeShort ((short) blockAlign);
        mo.writeShort (16);                           // bits
        mo.write ("data", 4);
        mo.writeInt ((int) claimedData);              // the lie: claimed data size
        if (actualDataBytes > 0)
        {
            std::vector<char> zeros ((size_t) actualDataBytes, 0);
            mo.write (zeros.data(), zeros.size());
        }

        auto file = juce::File::createTempFile (".wav");
        file.replaceWithData (mo.getData(), mo.getDataSize());
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
            // Duration scales with RCL_SOAK_SECONDS: a PR run does the quick fixed
            // pass (600 iters); the nightly TSan soak sets it to run for minutes,
            // giving races far more chances to interleave. See TESTING.md §3.
            const int soakSeconds = juce::SystemStats::getEnvironmentVariable ("RCL_SOAK_SECONDS", "0")
                                        .getIntValue();
            auto& params = proc->getParameters();

            auto oneStep = [&] (int i)
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
                juce::ignoreUnused (i);
            };

            if (soakSeconds > 0)
            {
                logMessage ("SOAK MODE: running concurrency race for " + juce::String (soakSeconds) + "s");
                const auto deadline = juce::Time::getMillisecondCounter() + (juce::uint32) soakSeconds * 1000;
                int i = 0;
                while (juce::Time::getMillisecondCounter() < deadline)
                    oneStep (i++);
            }
            else
            {
                for (int i = 0; i < 600; ++i)
                    oneStep (i);
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
// Sample load / decode fuzz. Decoding untrusted audio files is a real crash
// surface (it's exactly what a user dropping a random file exercises). Feeds
// SampleLoader::loadSample garbage, truncated, empty, and odd-but-valid files via
// the processor, then renders — so any bad buffer surfaces in audio too. The
// truncation cases keep a real header (which over-claims the sample count)
// against short data — exercising the read-past-EOF / claimed-vs-actual-length
// path that `static_cast<int>(reader->lengthInSamples)` rides on.
class SampleLoaderFuzzTests : public juce::UnitTest
{
public:
    SampleLoaderFuzzTests() : juce::UnitTest ("SampleLoaderFuzz") {}

    void runTest() override
    {
        juce::Random rng (0xF1E2D3);

        auto proc = std::make_unique<NewProjectAudioProcessor>();
        proc->prepareToPlay (48000.0, 256);

        auto renderABit = [&]
        {
            juce::AudioBuffer<float> buf (2, 256);
            for (int b = 0; b < 3; ++b)
            {
                buf.clear();
                juce::MidiBuffer midi;
                if (b == 0) midi.addEvent (juce::MidiMessage::noteOn (1, 48, 0.9f), 0);
                proc->processBlock (buf, midi);
                for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                    for (int s = 0; s < buf.getNumSamples(); ++s)
                        expect (std::isfinite (buf.getSample (ch, s)), "non-finite after load");
            }
        };

        beginTest ("garbage bytes with audio extensions never crash the loader");
        {
            // NOTE: we do NOT assert rejection. MP3 is sync-word based with no
            // container header, so the decoder leniently "decodes" random bytes as
            // noise rather than rejecting them (and may log its own internal
            // jassert on malformed frames — JUCE's domain, not ours). The contract
            // we enforce is: no crash, and whatever lands renders finite audio.
            const char* exts[] { ".wav", ".aiff", ".flac", ".ogg", ".mp3" };
            for (int t = 0; t < 60; ++t)
            {
                auto f = rcltest::writeGarbage (rng, exts[rng.nextInt (5)], rng.nextInt ({ 0, 8192 }));
                proc->sampleLoader.loadSample (rng.nextInt (4), f);
                {
                    const juce::ScopedLock sl (proc->getCallbackLock());
                    proc->rebuildLoadedIndices();
                }
                proc->sampleLoader.updateSynthesiserSounds();
                renderABit();
                f.deleteFile();
            }
        }

        beginTest ("empty files are rejected");
        {
            auto f = juce::File::createTempFile (".wav");
            f.replaceWithData ("", 0);
            expect (! proc->sampleLoader.loadSample (0, f), "empty file reported as loaded");
            renderABit();
            f.deleteFile();
        }

        beginTest ("truncations of a real WAV decode or reject without crashing");
        {
            juce::WavAudioFormat wav;
            auto whole = rcltest::writeAudioFile (rng, wav, ".wav", 2, 16, 44100.0, 22050);
            juce::MemoryBlock bytes;
            expect (whole.loadFileAsData (bytes), "could not read back generated WAV");
            expect (bytes.getSize() > 64, "generated WAV implausibly small");

            const int step = juce::jmax (1, (int) bytes.getSize() / 40);
            for (int cut = 0; cut <= (int) bytes.getSize(); cut += step)
            {
                auto t = juce::File::createTempFile (".wav");
                t.replaceWithData (bytes.getData(), (size_t) cut);
                proc->sampleLoader.loadSample (rng.nextInt (4), t);   // may pass or fail; must not crash
                {
                    const juce::ScopedLock sl (proc->getCallbackLock());
                    proc->rebuildLoadedIndices();
                }
                proc->sampleLoader.updateSynthesiserSounds();
                renderABit();
                t.deleteFile();
            }
            whole.deleteFile();
        }

        beginTest ("a header over-claiming the decoded length is rejected, not allocated");
        {
            // Claims 60M frames (> SampleSlot's 50M cap) but carries ~100 bytes of
            // real data. The decoded-length guard must reject this before allocating
            // the claimed ~120 MB read buffer. Robust to whether the WAV reader
            // trusts or clamps the header: either way, no crash and no huge alloc.
            auto f = rcltest::writeWavWithClaimedFrames (60000000LL, 100);
            const bool ok = proc->sampleLoader.loadSample (0, f);
            expect (! ok, "over-claiming header was accepted (guard not engaged)");
            renderABit();
            f.deleteFile();
        }

        beginTest ("odd-but-valid formats fold to mono, resample, and play");
        {
            // Generation is WAV/AIFF only (JUCE's own uncompressed writers). We do
            // NOT encode FLAC/OGG here: the product only DECODES them, and driving
            // the vendored libFLAC/Vorbis ENCODERS trips third-party UBSan findings
            // for code the plugin never runs. FLAC/OGG decoder reject-paths are
            // still covered by the garbage-bytes test above; valid-compressed-file
            // decode is left to pluginval + real-host testing (would otherwise need
            // checked-in binary fixtures).
            struct Spec { juce::String ext; int ch; int bits; double sr; int len; };
            const Spec specs[]
            {
                { ".wav",  1, 8,   8000.0,   1 },
                { ".wav",  2, 24,  192000.0, 7 },
                { ".wav",  6, 16,  48000.0,  512 },
                { ".wav",  8, 16,  44100.0,  256 },
                { ".aiff", 1, 16,  44100.0,  128 },
                { ".aiff", 2, 24,  96000.0,  64 },
            };

            for (const auto& s : specs)
            {
                std::unique_ptr<juce::AudioFormat> fmt;
                if (s.ext == ".wav") fmt = std::make_unique<juce::WavAudioFormat>();
                else                 fmt = std::make_unique<juce::AiffAudioFormat>();

                auto f = rcltest::writeAudioFile (rng, *fmt, s.ext, s.ch, s.bits, s.sr, s.len);
                if (f.getSize() == 0) { f.deleteFile(); continue; }   // combo unsupported by writer

                const bool ok = proc->sampleLoader.loadSample (0, f);
                expect (ok, "valid " + s.ext + " (" + juce::String (s.ch) + "ch/"
                              + juce::String (s.bits) + "bit) failed to load");
                {
                    const juce::ScopedLock sl (proc->getCallbackLock());
                    proc->rebuildLoadedIndices();
                    proc->reshuffleIndices();
                }
                proc->sampleLoader.updateSynthesiserSounds();

                // Flip the engine SR to force a resample of the just-loaded slot.
                proc->prepareToPlay (s.sr > 48000.0 ? 44100.0 : 96000.0, 256);
                renderABit();
                proc->prepareToPlay (48000.0, 256);

                f.deleteFile();
            }
        }

        proc->releaseResources();
    }
};

//==============================================================================
// Playback notifications cross from audio to UI without reading mutable pool state.
class PlaybackEventTests : public juce::UnitTest
{
public:
    PlaybackEventTests() : juce::UnitTest("Playback events") {}

    void runTest() override
    {
        NewProjectAudioProcessor proc;
        proc.prepareToPlay(44100.0, 128);
        juce::AudioBuffer<float> buffer(2, 128);
        juce::MidiBuffer midi;
        auto render = [&]
        {
            const juce::ScopedLock lock(proc.getCallbackLock());
            proc.processBlock(buffer, midi);
            midi.clear();
        };

        beginTest("Empty pool does not report playback");
        expectEquals(NewProjectAudioProcessor::getPlaybackEventSlot(proc.getPlaybackEvent()), -1);
        proc.requestTrigger();
        render();
        expect(proc.getPlaybackEvent() == 0);

        juce::Random rng(1234);
        const auto wav = rcltest::writeTempWav(rng, 4096);
        expect(proc.sampleLoader.loadSample(3, wav));
        proc.rebuildLoadedIndices();

        beginTest("MIDI and repeated Trigger hits publish distinct events for the same slot");
        midi.addEvent(juce::MidiMessage::noteOn(1, 72, 0.8f), 0);
        render();
        auto previous = proc.getPlaybackEvent();
        expectEquals(NewProjectAudioProcessor::getPlaybackEventSlot(previous), 3);
        proc.requestTrigger();
        render();
        expect(proc.getPlaybackEvent() != previous);
        expectEquals(NewProjectAudioProcessor::getPlaybackEventSlot(proc.getPlaybackEvent()), 3);

        beginTest("Idle, note-off, and Panic do not flash");
        previous = proc.getPlaybackEvent();
        render();
        expect(proc.getPlaybackEvent() == previous);
        midi.addEvent(juce::MidiMessage::noteOff(1, 72), 0);
        render();
        expect(proc.getPlaybackEvent() == previous);
        proc.requestTrigger();
        proc.requestPanic();
        render();
        expect(proc.getPlaybackEvent() == previous);

        beginTest("Audition reports the selected row and ignores empty or invalid slots");
        expect(proc.sampleLoader.loadSample(12, wav));
        proc.rebuildLoadedIndices();
        proc.auditionSample(12);
        expect(proc.getPlaybackEvent() != previous);
        expectEquals(NewProjectAudioProcessor::getPlaybackEventSlot(proc.getPlaybackEvent()), 12);
        previous = proc.getPlaybackEvent();
        proc.auditionSample(-1);
        proc.auditionSample(20);
        proc.auditionSample(0);
        expect(proc.getPlaybackEvent() == previous);

        beginTest("Series reports the slot actually selected, including gaps in the pool");
        auto* mode = proc.apvts.getParameter(ParameterIDs::playbackMode);
        mode->setValueNotifyingHost(0.0f);
        proc.resetPlaybackPosition();
        for (int expectedSlot : { 3, 12, 3 })
        {
            proc.requestTrigger();
            render();
            expectEquals(NewProjectAudioProcessor::getPlaybackEventSlot(proc.getPlaybackEvent()),
                         expectedSlot);
        }

        proc.releaseResources();
        wav.deleteFile();
    }
};

// Auto-registering instances.
static PlaybackEventTests        playbackEventTests;
static RandomizationEngineTests randomizationEngineTests;
static MidiMapperTests          midiMapperTests;
static ProcessorFuzzTests        processorFuzzTests;
static ProcessorAutomationTests  processorAutomationTests;
static ProcessorLifecycleTests   processorLifecycleTests;
static ProcessorConcurrencyTests processorConcurrencyTests;
static SampleLoaderFuzzTests     sampleLoaderFuzzTests;

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
