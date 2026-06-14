# Changelog

All notable changes to **Robin Control Lite** are documented here.
This project follows [semantic versioning](https://semver.org/).

---

## v1.0.1 — 2026-06-14

### Added
- **Drag-and-drop sample loading** — drop audio files directly into the sample
  pool from Finder / Explorer (or a DAW browser, where the host forwards it as a
  file drop). Behaves like the Load Samples button: additive, appending to the
  next empty slot. A highlight appears over the pool while you drag.

### Fixed
- Guard against invalid or zero-length audio files when loading samples.
- Guard against zero-sample processing blocks in the audio engine.
- Prevent double-registration of the MP3 format (could trip an assert / duplicate
  the reader).
- Hardened audio file writing for JUCE 8.0.4 compatibility.

### Changed
- Build system: macOS-only plugin auto-copy is now gated to Apple, so Windows
  builds configure cleanly. *(No effect on the installed plugin.)*

---

## v1.0.0 — 2026-04-26

First public release. A free monophonic sampler for macOS (VST3 / AU) and
Windows (VST3).

### Features
- **20 sample slots** — load WAV, AIFF, FLAC, OGG, and MP3 (decode) files.
- **Monophonic playback** — one voice at a time, triggered from any MIDI key.
- **Round-robin modes** — Series (cycle in order) and Random (no repeats until
  the pool is exhausted).
- **Per-note randomization** — independent negative/positive ranges below every
  knob for natural, non-looped variation.
- **Sample shaping** — start/end trim, global pitch (semitones + fine tune),
  and tone control (low + high shelf).
- **Amplitude** — volume and pan.
- **Trigger** — fire the next sample without MIDI input.
- **Panic** — stop all sound instantly.
- **Sample pool management** — audition, reorder, replace, delete, and clear.

### Distribution
- macOS installer signed and notarized by Apple.
- Windows installer (VST3).
