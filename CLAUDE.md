# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**Robin Control Lite** is a free monophonic sampler plugin (VST3 / AU / Standalone, AAX in v1.1) by `conduit.dsp`. Built with JUCE 8+ and C++17.

As of 2026-04-25 this folder is the **authoritative project**. It was originally a UI-redesign sandbox forked from `../round-robin-lite`, but has been promoted to be the main codebase. The sibling folder `../round-robin-lite` is retired — kept on disk as read-only legacy reference, not built or installed.

For the full development plan (formats, signing, distribution, CI, testing, license decisions), see `spec.md` at the repo root. **`spec.md` is the source of truth** for anything not covered here. Sibling docs at the repo root:
- `release-spec.md` — **the active v1.0 ship execution playbook**. Sequential, command-level checklist with [x]/[ ] checkboxes covering build config, Apple signing, AAX validation, Pro Tools test, Windows build, distribution. When `spec.md` and `release-spec.md` disagree on something v1.0-specific, `release-spec.md` wins. Update it as you complete items.
- `spec-paperwork.md` — admin/legal checklist (Apple Developer enrollment, trademark, etc.)
- `EULA.md` — end-user license for the binary; establishes **Conduit DSP LLC** (Kingston, NY; Ulster County jurisdiction) as the legal entity. Effective date: April 26, 2026 (v1.0). Will be published at `https://conduitdsp.com/eula/robin-control-lite/`.
- `LICENSE` — source code license: **All Rights Reserved (proprietary)**. The source is not open-source; a Pro version is planned, so the codebase must remain shareable between Lite and Pro without a third-party fork ever shipping.
- `Privacy.md` — short pointer to the canonical privacy policy at `https://conduitdsp.com/privacy-policy/`. The plugin itself collects nothing; the website handles email-capture (MailerLite) for downloads.
- `README.md` — end-user-facing install/use docs

GitHub remote: `https://github.com/ahamadey27/robin-control-lite.git` (private). Local folder name (`robin-control-redesign`) intentionally not changed — VSCode workspace paths still point at the old folder name.

### Product behavior (free version)

The shipped product is **monophonic** and triggers from **any MIDI key**. There is **no** paired-key MIDI mapping, **no** white-keys-only restriction, and **no** chromatic per-key pitching. Pitch is global only (semitone + fine-tune knobs). The codebase still contains paired-key infrastructure (`MidiMapper::NUM_KEY_PAIRS = 10`, `RRSound::keyPairIndex`) inherited from the parent project — that's internal plumbing, not user-facing behavior. **When the inherited code description disagrees with the shipped product, the product wins.** See `memory/project_free_version_scope.md` for full reasoning.

## Build

```bash
cd NewProject
cmake -B build
cmake --build build
```

JUCE resolution: defaults to `~/Documents/JUCE` if present, else `FetchContent` pulls JUCE 8.0.4. Override the local path with `cmake -DJUCE_PATH=/path/to/JUCE -B build`.

Outputs copy automatically after every build:
- VST3 → `~/Library/Audio/Plug-Ins/VST3/Robin Control Lite.vst3`
- AU → `~/Library/Audio/Plug-Ins/Components/Robin Control Lite.component`

AAX is built when the SDK is found at `~/SDKs/aax-sdk-2-9-0` (override with `cmake -DJUCE_AAX_SDK_PATH=...`). AAX has no auto-copy — the eval `.aaxplugin` lives in `build/RobinControlLite_artefacts/<Config>/AAX/` and you copy it manually to `/Library/Application Support/Avid/Audio/Plug-Ins/` for Pro Tools Developer testing.

Standalone was dropped from v1.0 ship (see `release-spec.md` §2.1). For local plugin debugging, `.vscode/launch.json` now uses **attach-to-process** — build the plugin, open the AU/VST3 in any DAW, then run the launch config and pick the host's process from the picker.

The build is configured `CMAKE_BUILD_TYPE=Debug` (single-config Unix Makefiles). JUCE writes per-format artefacts under `build/RobinControlLite_artefacts/Debug/<Format>/`.

Xcode project lands at `NewProject/build/RobinControlLite.xcodeproj`.

A Projucer file (`NewProject/RobinControlLite.jucer`) is kept in sync with CMake, but **CMake is the authoritative build**. The old `NewProject/Builds/` tree is stale Projucer output — do not rely on it; safe to delete.

### Testing

A layered automated testing platform exists — **`TESTING.md` is the source of truth**. Quick reference:
- `scripts/test-plugin.sh` — local pluginval (strictness 10) + auval. Run before every release.
- `tests/` — JUCE `UnitTest` console app for pure-logic units (RandomizationEngine, MidiMapper). Build with `cmake -DRCL_BUILD_TESTS=ON` (off by default; does not affect the plugin build).
- `scripts/test-sanitizers.sh` — ASan/UBSan + TSan over the test target (Clang/macOS only).
- `.github/workflows/validate.yml` — CI on every push: builds + validates on **macOS and Windows**, plus unit-test and sanitizer jobs. **Green as of 2026-06-01.**

Still useful for manual checks: JUCE AudioPluginHost or any DAW (the real-host smoke matrix in `TESTING.md §5`). **Next testing increment:** grow `tests/` to instantiate `NewProjectAudioProcessor` and fuzz `processBlock`/`setStateInformation` so sanitizers cover the crash-class paths. The whole platform is plugin-agnostic and designed to be lifted into the Pro version (`TESTING.md §7`).

### Audio formats supported

WAV, AIFF, FLAC, OGG, **MP3** (decode-only). MP3 is gated behind `JUCE_USE_MP3AUDIOFORMAT=1` in `CMakeLists.txt` (`target_compile_definitions`) — JUCE 8's basic format pack does NOT include it by default. `MP3AudioFormat` is also explicitly registered in `PluginProcessor` after `formatManager.registerBasicFormats()`. JUCE's MP3 decoder ships royalty-free; safe for distribution. The picker filters and `SampleLoader::supportedFormats` array must list `.mp3` too — all three places stay in sync.

### Plugin identity (for DAW registry)

| Field | Value |
|---|---|
| Target / project | `RobinControlLite` |
| Product name | `Robin Control Lite` |
| Company | `conduit.dsp` |
| Plugin code | `rcll` |
| Manufacturer code | `Cdsp` |
| Bundle ID | `dsp.conduit.RobinControlLite` |

## Architecture

Standard JUCE Synthesiser pattern:

- **PluginProcessor** (`Source/PluginProcessor.h/cpp`) — Main audio engine. Owns the `juce::Synthesiser`, `AudioProcessorValueTreeState` (APVTS), DSP processors, `SampleSlot[20]` array, and `SampleLoader`. `processBlock()` renders audio and applies the DSP chain.
- **PluginEditor** (`Source/PluginEditor.h/cpp`) — GUI. Custom Look-and-Feel classes for knobs/sliders. 1400×400px canvas. Header hosts (right-to-left): About `?`, Panic `!`, Trigger, Save, Load.
- **RRVoice** (`Source/Audio/RRvoice.h/cpp`) — `SynthesiserVoice` subclass. Handles sample playback, pitch shifting, and per-note randomization. The `juce::ADSR` member is retained but dormant in Lite.
- **RRSound** (`Source/Audio/RRSound.h/cpp`) — `SynthesiserSound` subclass. Holds one audio buffer per sample.
- **MidiMapper** (`Source/Audio/MidiMapper.h`) — Static utility mapping 10 key pairs to MIDI notes. Root note = C1 (MIDI 36).
- **SampleSlot/SampleLoader** (`Source/Data/`) — Sample loading with stereo-to-mono conversion and resampling.
- **RandomizationEngine** (`Source/DSP/RandomizationEngine.h/cpp`) — Asymmetric per-note randomization (separate negative/positive ranges).
- **ParametersIDs** (`Source/Parameters/ParametersIDs.h`) — All 47 parameter ID string constants.
- **SampleManagerPanel** (`Source/UI/SampleManagerPanel.h/cpp`) — Left panel component with sample list, load/delete/replace/audition/reorder, playback toggle.
- **DualThumbRndSlider** (`Source/UI/DualThumbRndSlider.h`) — Header-only dual-thumb horizontal slider for neg/pos randomization. Reads/writes hidden APVTS sliders.

The `Source/` tree was originally copied from `../round-robin-lite/NewProject/Source/`. Class names (e.g. `NewProjectAudioProcessor`, `roundRobinIndex`) still reflect that origin. Renaming them is a non-goal unless the user asks.

### DSP Chain (`Source/DSP/`)

- **ToneControl** — Simple 2-knob low/high shelf EQ (250Hz/4kHz, ±12dB). Fully integrated. Caches last-applied `(lowGain_dB, highGain_dB)` and short-circuits `updateFilters()` when neither moved (epsilon 0.001f) — coefficient math is the costly part. **Don't remove this cache** without measuring; it's the largest single CPU win in idle. The cache is invalidated in `prepareToPlay` (sample-rate change must rebuild).
- **ThreeBandEQ** — Full 3-band EQ. Code exists but is commented out (reserved for Pro).
- **TransientShaper** — Attack/decay transient processor. Also commented out.

### UI (`Source/UI/`)

- **RRLookAndFeel** — Custom `LookAndFeel_V4` subclasses: `RRKnobLAF`, `RRNegSliderLAF` / `RRPosSliderLAF`, `RRToggleLAF`, `RRButtonLAF`.

### Section-label visual treatment (preserve)

Every section header text — **SAMPLE POOL**, **RANDOM ALGORITHM**, **AMPLITUDE**, **TONE**, **PITCH**, **SAMPLE START/END** — uses a unified treatment:
1. Cream `0xffece5d4` 1px drop-shadow underneath (alpha 0.55)
2. Dark `0xff0a0806` 1px outline stroke
3. Vertical gradient fill on the path: `accent.brighter(0.15f)` at top → `accent.darker(0.10f)` at bottom (S612 button-strip technique)
4. Faint white sheen along glyph tops: 0.8px tall rect at 25% alpha, clipped to the text path

Helper: `drawSectionTitle` lambda in `PluginEditor.cpp::paint`. Sibling implementation in `SampleManagerPanel.cpp::paint` for SAMPLE POOL. **Don't simplify or remove** any of the four layers above — the user explicitly approved this look on 2026-04-25 after several iterations. SAMPLE POOL position is hard-locked at y=3.0f (see `memory/feedback_sample_pool_text_locked.md`).

## Key Design Decisions

- **Monophonic:** one voice at a time.
- **Unpitched playback:** MIDI key selects sample, not pitch. Pitch is global (semitone + fine tune).
- **Playback modes:** Series (round-robin) or Random (Fisher-Yates, no repeats until all played).
- **Randomization:** every parameter has 4 rnd params (neg/pos range). Values generated per note-on in `RRVoice::startNote()`.
- **Sample Start/End:** percentages reference `maxPoolSampleLength` then clamp per-voice. **Asymmetric edge case (intentional):** if Start (in pool-relative samples) lands past *this* sample's end, the voice falls back to applying the Start percentage to this sample's own length so short samples don't go silent in mixed-duration pools (RRvoice.cpp `startNote`). End uses straight clamp — short samples play fully when End extends past them. This was a fix for ticks 15–17 of the Random Algorithm table (the only ticks with `sampleStartRndPos > 0`), where short WAVs went silent in pools that included multi-minute MP3s.
- **Trigger / Panic:** both are atomic flags consumed in `processBlock`. Panic wins over Trigger queued in the same block.
- **Audio/message-thread safety contract:** any code path that mutates `sampleSlots[]`, `loadedSlotIndices`, `shuffledIndices`, `roundRobinIndex`, or the synthesiser's `RRSound` from the message thread MUST take `getCallbackLock()` (`juce::ScopedLock`). The audio thread holds it implicitly during `processBlock`, so this serializes cleanly. `SampleLoader` takes a `const juce::CriticalSection&` reference at construction (passed `getCallbackLock()`) and locks internally in `loadSample`/`clearSlot`/`setSampleRate`/`updateSynthesiserSounds`. `PluginProcessor::swapSamples`/`insertSample`/`auditionSample`/`resetPlaybackPosition` and the slot-restore block in `setStateInformation` lock at the entry. Decode/resample happens off-lock; only the final swap is locked, so big files don't stall `processBlock`.
- **`prepareToPlay` rebuilds after resample.** `sampleLoader.setSampleRate()` resamples every loaded slot to the new rate, which changes each slot's sample count. `rebuildLoadedIndices()` MUST be called immediately after, otherwise `maxSampleLength` stays stale at the old rate and voices clamp playback to that — causing tail cutoff at 88.2k/96k. Already wired in `prepareToPlay`; keep it that way.
- **Param IDs are frozen.** Once v1.0 ships, renaming a param ID in `ParametersIDs.h` breaks every saved preset. Migrate via `setStateInformation`, don't rename in place.

## Visual references

- **`Images/S612_*`** — primary aesthetic reference (Akai S612 sampler-inspired warm cream lane).
- **`Images/HiFi_*`, `Images/Sampler_01.png`** — secondary inspiration only.
- **`LOGOS/FULLLOGO-email-white.png`** — the in-app brand mark; keyed to alpha at runtime. The "clear" variant is for marketing/web, not the binary.
- The layout lives in `PluginEditor.cpp` — `paint()` and `resized()` constants are duplicated; keep them in sync.

## Randomization UI

Current design:
- **DualThumbRndSlider** — horizontal bar below each knob, two thumbs (neg left, pos right), 4px gap at 0%.
- **Thin arc outlines** — drawn in `paintOverChildren()`, 2px stroke at 70% alpha, CCW neg / CW pos.
- Hidden APVTS sliders store the values; DualThumbRndSlider reads/writes them directly.

## "Undo to original"

When the user says **"undo to original"** (or any close variant — "revert to original", "back to original", etc.), treat it as a command to restore the project to the baseline state captured on 2026-04-22: git commit `5843ed5` plus the CLAUDE.md edits made that same day. The `original-baseline` git tag marks this state. **Note:** this baseline predates the 2026-04-25 pivot to main project — restoring it reverts identity strings, the spec.md, and CLAUDE.md back to the sandbox era. Confirm with the user before committing or discarding anything else.

```bash
git -C /Users/alex/Documents/Github/robin-control-redesign checkout original-baseline -- .
```

Do NOT interpret "undo to original" as reverting to the initial commit (`9ab5621`) or to any earlier state — it always means this baseline.

## Working on this project

The "design sandbox" constraints from the previous era are **lifted**. You may now:
- Move, reorder, or restructure controls — the layout is no longer frozen.
- Touch the audio engine (PluginProcessor, RRvoice, DSP/, Data/) directly. Changes no longer need to be ported from `../round-robin-lite`; that sibling is legacy.
- Refactor freely as long as preset state (param IDs, APVTS structure) stays compatible once v1.0 ships.

What still applies:
- **Param IDs frozen post-v1.0** (preset compatibility). Pre-v1.0 they're still mutable.
- **CMake is authoritative**, Projucer file is sync-only.
- For anything about formats, signing, distribution, CI, testing, or licensing decisions, defer to `spec.md`.
- **"Load Samples" is additive, not destructive.** Both the header `Load Samples` button and the in-pool "click to add (x) samples" placeholder route through `addMoreSamples()` and append to the next empty slot. Users clear the pool with the Clear button to start fresh. Don't restore the old replace-everything behavior — the user explicitly chose this on 2026-04-27 because the old replace was hostile UX (had to reload the entire pool just to add one more sample).
