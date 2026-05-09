# Robin Control Lite

A free monophonic sampler plugin by [conduit.dsp](https://conduitdsp.com). Drop in up to 20 audio samples, trigger them from any key on your MIDI controller, and shape them with built-in tone, randomization, and round-robin playback.

[![JUCE](https://img.shields.io/badge/JUCE-8.0+-blue.svg)](https://juce.com/)
[![Formats](https://img.shields.io/badge/Formats-VST3%20%7C%20AU-green.svg)]()
[![Platforms](https://img.shields.io/badge/Platforms-macOS%20%7C%20Windows-lightgrey.svg)]()

> Status: pre-release. v1.0 ships when everything in [`spec.md`](spec.md) §7.11 is green.

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

1. Download `RobinControlLite-x.y.z.pkg` from the latest release.
2. Run the installer. It places:
   - VST3 → `/Library/Audio/Plug-Ins/VST3/Robin Control Lite.vst3`
   - AU → `/Library/Audio/Plug-Ins/Components/Robin Control Lite.component`
3. Rescan plugins in your DAW.

The installer is signed and notarized by Apple. If macOS still flags it, right-click the `.pkg` → Open → confirm.

### Windows (10 1809+ / 11)

1. Download `RobinControlLite-x.y.z-setup.exe` from the latest release.
2. Run the installer. It places:
   - VST3 → `C:\Program Files\Common Files\VST3\Robin Control Lite.vst3`
3. Rescan plugins in your DAW.

Windows SmartScreen may show a warning on first run. Click "More info" → "Run anyway" if you trust the publisher (`conduit.dsp`).

### Tested DAWs

Tier 1 (verified before every release): Logic Pro, Ableton Live 12, Reaper 7, FL Studio 21.
Tier 2 (best-effort): Cubase 13, Studio One 6, Bitwig Studio, GarageBand, MainStage.
Pro Tools (AAX) support is planned for v1.1.

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

This repo is the authoritative codebase for Robin Control Lite. The full development plan (build, formats, signing, CI, testing strategy, license decisions) lives in [`spec.md`](spec.md). Architecture and conventions for working in the source tree live in [`CLAUDE.md`](CLAUDE.md).

### Build from source

```bash
cd NewProject
cmake -B build
cmake --build build --config Release
```

JUCE resolves to `~/Documents/JUCE` if present, else `FetchContent` pulls JUCE 8.0.4. Override with `cmake -DJUCE_PATH=/path/to/JUCE -B build`.

Built artifacts land in `NewProject/build/RobinControlLite_artefacts/Release/{VST3,AU}/` and are auto-copied to the system plugin folders for testing. AAX (when the SDK is present at `~/SDKs/aax-sdk-2-9-0`) lands in the same tree but isn't auto-copied — see [`CLAUDE.md`](CLAUDE.md) for the manual copy path. See [`spec.md`](spec.md) §3 for the full build details and §5 for signing/notarization.

### License

The plugin binary is distributed free of charge. Source license: see `LICENSE` (TBD — pending decision in `spec.md` §1.4).

JUCE itself is used under the JUCE Personal license (pending confirmation; see `spec.md` §1.2).

VST is a trademark of Steinberg Media Technologies GmbH. Audio Units is a trademark of Apple Inc.

---

**conduit.dsp** — `hello@conduitdsp.com`
