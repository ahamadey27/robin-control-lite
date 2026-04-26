# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**Robin Control Lite** is a free monophonic sampler plugin (VST3 / AU / Standalone, AAX in v1.1) by `conduit.dsp`. Built with JUCE 8+ and C++17.

As of 2026-04-25 this folder is the **authoritative project**. It was originally a UI-redesign sandbox forked from `../round-robin-lite`, but has been promoted to be the main codebase. The sibling folder `../round-robin-lite` is retired — kept on disk as read-only legacy reference, not built or installed.

For the full development plan (formats, signing, distribution, CI, testing, license decisions), see `spec.md` at the repo root. **`spec.md` is the source of truth** for anything not covered here. Sibling docs at the repo root:
- `spec-paperwork.md` — admin/legal checklist (Apple Developer enrollment, trademark, etc.)
- `EULA.md` — end-user license; establishes **Conduit DSP LLC** (Kingston, NY; Ulster County jurisdiction) as the legal entity
- `README.md` — end-user-facing install/use docs

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
- Standalone → `/Applications/Robin Control Lite.app` (post-build hook in CMakeLists; JUCE has no `STANDALONE_COPY_DIR` so we wired one manually)

Xcode project lands at `NewProject/build/RobinControlLite.xcodeproj`.

A Projucer file (`NewProject/RobinControlLite.jucer`) is kept in sync with CMake, but **CMake is the authoritative build**. The old `NewProject/Builds/` tree is stale Projucer output — do not rely on it; safe to delete.

No automated tests yet. Test manually via JUCE AudioPluginHost or any DAW. `pluginval` is on the roadmap (see spec.md §7).

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

- **ToneControl** — Simple 2-knob low/high shelf EQ (250Hz/4kHz, ±12dB). Fully integrated.
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
- **Sample Start/End:** percentages reference `maxPoolSampleLength` then clamp per-voice.
- **Trigger / Panic:** both are atomic flags consumed in `processBlock`. Panic wins over Trigger queued in the same block.
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
