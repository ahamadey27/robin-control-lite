# Robin Control Lite

A free monophonic sampler plugin by [conduit.dsp](https://conduitdsp.com). Drop in up to 20 audio samples, trigger them from any key on your MIDI controller, and shape them with built-in tone, randomization, and round-robin playback.

[![JUCE](https://img.shields.io/badge/JUCE-8.0+-blue.svg)](https://juce.com/)
[![Formats](https://img.shields.io/badge/Formats-VST3%20%7C%20AU-green.svg)]()
[![Platforms](https://img.shields.io/badge/Platforms-macOS%20%7C%20Windows-lightgrey.svg)]()

> This checkout is preparing **2.0.0**, which is not yet published. See
> [`FINAL_RELEASE_2.0.0.md`](FINAL_RELEASE_2.0.0.md) for current release status.

---

## What it does

- **20 sample slots** — load `.wav`, `.aif`, `.flac`, `.ogg`, `.mp3` via the file picker
- **Monophonic playback** — one voice at a time, triggered from any MIDI key (no chromatic pitching in the free version — every key plays back at the sample's natural pitch, with global semitone + fine-tune offsets)
- **Round-robin playback** — Series mode (cycle in order) or Random mode (no repeats until pool exhausted)
- **Per-note randomization** — every parameter has independent negative and positive randomization ranges
- **Sample shaping** — start/end trim, global pitch (semitones + cents), tone control (low + high shelf)
- **Trigger + Panic** — fire the next round-robin sample without a MIDI input; cut all voices instantly

Built for footsteps, foley, and any percussive sample-set work where slight per-note variation makes the difference between believable and obviously-looped.

---

## Install

### macOS (11.0 Big Sur or newer, Apple Silicon + Intel)

1. Obtain `Robin Control Lite 2.0.0.pkg` and a free Moonbase license when this release is made available.
2. Run the installer. It places:
   - VST3 → `/Library/Audio/Plug-Ins/VST3/Robin Control Lite.vst3`
   - AU → `/Library/Audio/Plug-Ins/Components/Robin Control Lite.component`
   - AAX → `/Library/Application Support/Avid/Audio/Plug-Ins/Robin Control Lite.aaxplugin`
3. Rescan plugins in your DAW.

The final installer is Developer ID signed; see the release record for notarization evidence. Open the plugin and use **Activate** to sign into the account holding the free license. Online activations allow up to 90 days offline; permanent offline activation is a separate file exchange. Trials are disabled. Customer iLok activation is not required for Lite.

### Windows (10 1809+ / 11)

1. Download `RobinControlLite-x.y.z-setup.exe` from the latest release.
2. Run the installer. It places:
   - VST3 → `C:\Program Files\Common Files\VST3\Robin Control Lite.vst3`
3. Rescan plugins in your DAW.

Windows v2.0.0 final build, signing, packaging and host validation remain pending. The steps above describe the existing VST3 distribution; see the Windows build guide for producing the new version.

### Tested DAWs

Planned Tier 1 release matrix: Logic Pro, Ableton Live 12, Reaper 7, FL Studio 21.
Tier 2 (best-effort): Cubase 13, Studio One 6, Bitwig Studio, GarageBand, MainStage.
The macOS 2.0.0 AAX candidate is signed and user-confirmed working in regular
Pro Tools / Intro. The final Mac installer includes AAX. Previous host acceptance does not replace
testing the new licensed final binaries. Windows AAX validation remains pending.

Full host matrix in [`spec.md`](spec.md) §7.3.

---

## Quick start

1. Click **Load Samples** in the header (or the "click to add samples" placeholder in the empty pool) and pick up to 20 audio files
2. Press any key on your MIDI controller — samples trigger monophonically (one voice at a time)
3. Adjust pitch, tone, sample start/end on the main panel
4. Tweak the randomization range below each knob to add per-note variation
5. Switch playback mode (Series ↔ Random) in the header

### Tip: audition samples before loading

The Load Samples dialog is the macOS / Windows native file picker, which has built-in audio preview:

- **macOS** — switch the dialog to **Column view** (the rightmost view button, or `⌘ 3`) to get a play button on each audio file. Or select a file and press **Space** to open QuickLook with playback.
- **Windows** — select a file in the dialog and use the **Preview pane** (View → Preview pane in the dialog's toolbar) to scrub and play.

Once samples are in the pool, click the speaker icon on any slot to audition it through the plugin's own engine (with your current tone / pitch / start-end settings applied).

---

## Reporting issues

- Bug reports: open an issue on this repo with host + version, OS + version, and repro steps
- Support email: `hello@conduitdsp.com`

---

## For developers

This repo is the authoritative codebase for Robin Control Lite. Architecture and
working conventions live in [`AGENTS.md`](AGENTS.md); the broader plan is in
[`spec.md`](spec.md). Current build and release handoffs:

- [macOS 2.0.0 final release build](FINAL_RELEASE_2.0.0.md)
- [AAX compilation, PACE signing, validation, and installation](AAX_BUILD_AND_SIGNING.md)
- [Clone and build on Windows, including AAX](WINDOWS_BUILD_AND_AAX.md)

### Build from source

```bash
cd NewProject
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build --config Release
```

JUCE resolves to `~/Documents/JUCE` if present, else `FetchContent` downloads checksum-pinned JUCE 8.0.15. Override with `cmake -DJUCE_PATH=/path/to/JUCE -B build`.

Built artifacts land under `NewProject/build/RobinControlLite_artefacts/Release/`.
Local macOS builds copy VST3/AU to user plug-in folders unless
`RCL_COPY_PLUGIN_AFTER_BUILD=OFF`; Windows builds do not auto-install.
For the reproducible macOS 2.0.0 candidate, use the release command linked above.
AAX is SDK-gated: explicitly build `RobinControlLite_AAX`, then follow the AAX
handoff to sign and validate before retail Pro Tools installation. The Windows
guide uses explicit SDK paths and a fresh x64 build directory.

### License

The plugin binary is distributed free of charge. Source is proprietary, All Rights Reserved; see `LICENSE`.

JUCE itself is used under the JUCE Personal license (pending confirmation; see `spec.md` §1.2).

VST is a trademark of Steinberg Media Technologies GmbH. Audio Units is a trademark of Apple Inc.

---

**conduit.dsp** — `hello@conduitdsp.com`
