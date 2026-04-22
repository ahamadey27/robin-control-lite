# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**Robin Control Redesign** is a sandbox for exploring alternate UI / control layouts for the Round Robin Lite sampler plugin. The audio engine is an exact copy of `round-robin-lite` (sibling folder `../round-robin-lite`); the codebase here exists so UI experiments can evolve without touching the shipping product. Built with JUCE 8+ and C++17. The CMake/Projucer targets have been renamed to `robindesign` so the two plugins can coexist in the same DAW plugin registry without colliding.

## Build

Requires JUCE installed at `/Users/alex/Documents/JUCE` (hardcoded in `NewProject/CMakeLists.txt`).

```bash
cd NewProject
cmake -B build
cmake --build build
```

Output copies automatically to `~/Library/Audio/Plug-Ins/VST3/robin-design.vst3`. Xcode project lands at `NewProject/build/robindesign.xcodeproj`.

Formats: VST3, AU, Standalone. A Projucer file (`NewProject/robindesign.jucer`) also exists and is kept in sync with CMake, but CMake is the authoritative build. The old `NewProject/Builds/MacOSX/` tree is stale Projucer output from the parent project — do not rely on it.

No automated tests. Test manually via JUCE AudioPluginHost or any DAW.

### Plugin identity (for DAW registry)

| Field | Value |
|---|---|
| Target / xcodeproj | `robindesign` |
| Product name | `robin-design` |
| Company | `robindesign` |
| Plugin code | `rdsn` |
| Manufacturer code | `Rdsn` |

## Relationship to `round-robin-lite`

- The `Source/` tree was copied verbatim from `../round-robin-lite/NewProject/Source/` at project init.
- Expect them to drift: this project is where UI redesigns happen; the sibling is where stable work lives.
- If you need reference material about the original design (spec, image refs), look in `../round-robin-lite/` rather than duplicating files here.
- Class names, param IDs, and variable names (e.g. `NewProjectAudioProcessor`, `roundRobinIndex`) still reflect the origin — renaming them is a non-goal unless the user asks.

## Architecture

Standard JUCE Synthesiser pattern:

- **PluginProcessor** (`Source/PluginProcessor.h/cpp`) — Main audio engine. Owns the `juce::Synthesiser`, `AudioProcessorValueTreeState` (APVTS), DSP processors, `SampleSlot[20]` array, and `SampleLoader`. `processBlock()` renders audio and applies the DSP chain.
- **PluginEditor** (`Source/PluginEditor.h/cpp`) — GUI. Custom Look-and-Feel classes for knobs/sliders. 1400×400px canvas. Header hosts (right-to-left): About `?`, Panic `!`, Trigger, Save, Load. **This is the primary file for redesign work.**
- **RRVoice** (`Source/Audio/RRvoice.h/cpp`) — `SynthesiserVoice` subclass. Handles sample playback, pitch shifting, and per-note randomization. The `juce::ADSR` member is retained but dormant in Lite.
- **RRSound** (`Source/Audio/RRSound.h/cpp`) — `SynthesiserSound` subclass. Holds one audio buffer per sample.
- **MidiMapper** (`Source/Audio/MidiMapper.h`) — Static utility mapping 10 key pairs to MIDI notes. Root note = C1 (MIDI 36).
- **SampleSlot/SampleLoader** (`Source/Data/`) — Sample loading with stereo-to-mono conversion and resampling.
- **RandomizationEngine** (`Source/DSP/RandomizationEngine.h/cpp`) — Asymmetric per-note randomization (separate negative/positive ranges).
- **ParametersIDs** (`Source/Parameters/ParametersIDs.h`) — All 47 parameter ID string constants.
- **SampleManagerPanel** (`Source/UI/SampleManagerPanel.h/cpp`) — Left panel component with sample list, load/delete/replace/audition/reorder, playback toggle.
- **DualThumbRndSlider** (`Source/UI/DualThumbRndSlider.h`) — Header-only dual-thumb horizontal slider for neg/pos randomization. Reads/writes hidden APVTS sliders.

### DSP Chain (`Source/DSP/`)

- **ToneControl** — Simple 2-knob low/high shelf EQ (250Hz/4kHz, ±12dB). Fully integrated.
- **ThreeBandEQ** — Full 3-band EQ. Code exists but is commented out (reserved for Pro).
- **TransientShaper** — Attack/decay transient processor. Also commented out.

### UI (`Source/UI/`)

- **RRLookAndFeel** — Custom `LookAndFeel_V4` subclasses: `RRKnobLAF`, `RRNegSliderLAF` / `RRPosSliderLAF`, `RRToggleLAF`.

## Key Design Decisions (inherited)

- **Monophonic:** one voice at a time.
- **Unpitched playback:** MIDI key selects sample, not pitch. Pitch is global (semitone + fine tune).
- **Playback modes:** Series (round-robin) or Random (Fisher-Yates, no repeats until all played).
- **Randomization:** every parameter has 4 rnd params (neg/pos range). Values generated per note-on in `RRVoice::startNote()`.
- **Sample Start/End:** percentages reference `maxPoolSampleLength` then clamp per-voice.
- **Trigger / Panic:** both are atomic flags consumed in `processBlock`. Panic wins over Trigger queued in the same block.

## Working on the redesign

- **The layout lives in `PluginEditor.cpp`.** The `paint()` and `resized()` layout constants are duplicated across both methods — keep them in sync.
- The audio-engine files (PluginProcessor, RRvoice, DSP/, Data/) should generally NOT be touched during UI redesigns — changes there should be ported from / to `round-robin-lite` instead.
- Param IDs are frozen (preset compatibility with the sibling project). Don't rename them in `ParametersIDs.h`.
- User-visible identity strings say "Robin Design" — header in `PluginEditor.cpp` (split "Robin" / "Design"), About dialog in `PluginEditor.h`, init log in `PluginProcessor.cpp`. Update these if the redesign takes on yet another name.

## Randomization UI

The current design uses:
- **DualThumbRndSlider** — horizontal bar below each knob, two thumbs (neg left, pos right), 4px gap at 0%.
- **Thin arc outlines** — drawn in `paintOverChildren()`, 2px stroke at 70% alpha, CCW neg / CW pos.
- Hidden APVTS sliders store the values; DualThumbRndSlider reads/writes them directly.

Alternatives to this pattern are fair game in this project.
