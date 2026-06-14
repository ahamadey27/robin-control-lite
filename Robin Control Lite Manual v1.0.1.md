# Robin Control Lite — Manual

**Version 1.0.1** · by conduit.dsp

---

Robin Control Lite is a free monophonic sampler. Load your samples, play them
from any MIDI key, and add subtle per-note variation so repeated hits never
sound identical.

---

## 1. Install

### macOS (11.0 or newer)

1. Download `RobinControlLite-1.0.1.pkg`.
2. Run the installer.
3. Restart your DAW and rescan plugins.

Installs to:
- VST3 → `/Library/Audio/Plug-Ins/VST3/`
- AU → `/Library/Audio/Plug-Ins/Components/`

### Windows (10 or 11)

1. Download `RobinControlLite-1.0.1-setup.exe`.
2. Run the installer.
3. Restart your DAW and rescan plugins.

Installs to:
- VST3 → `C:\Program Files\Common Files\VST3\`

---

## 2. Quick start

1. **Load samples** — Click **Load Samples** in the header (or the placeholder
   in the empty pool) and pick your audio files.
2. **Play** — Press any key on your MIDI controller. Samples play one at a time.
3. **Shape** — Adjust pitch, tone, and start/end on the main panel.
4. **Vary** — Set a randomization range below any knob for per-note variation.
5. **Choose a mode** — Switch between **Series** and **Random** in the header.

---

## 3. The header

| Button | What it does |
|---|---|
| **Load Samples** | Add audio files to the pool (adds to existing — does not replace). You can also drag samples straight into the pool from your DAW or the OS file browser (Finder / Explorer). |
| **Save** | Save your current settings as a preset. |
| **Trigger** | Fire the next sample without a MIDI key. |
| **Panic** `!` | Instantly stop all sound. |
| **About** `?` | Version and credits. |
| **Series / Random** | Choose how samples are picked (see Section 5). |

---

## 4. The sample pool

- Holds up to **20 samples**.
- Supported formats: **WAV, AIFF, FLAC, OGG, MP3**.
- **Drag and drop** — drop audio files directly into the pool from your DAW or the OS file browser (Finder / Explorer).
- **Speaker icon** — audition a sample through the plugin.
- **Drag** a slot to reorder it.
- **Delete / replace** a slot from its controls.
- **Clear** empties the pool so you can start fresh.

**Tip — preview before loading:** The Load Samples window is your system's
native file picker, which can play audio.
- *macOS:* switch to Column view (`⌘3`), or select a file and press **Space**.
- *Windows:* turn on the **Preview pane** in the dialog toolbar.

---

## 5. Playback modes

- **Series** — Samples play in order, cycling back to the start (round-robin).
- **Random** — Samples play in random order, with no repeats until every sample
  in the pool has played once.

Playback is **monophonic**: one sample sounds at a time. Each key triggers the
next sample — it does **not** change the pitch.

---

## 6. The controls

| Control | What it does |
|---|---|
| **Pitch — Semitones** | Shift all samples up or down in semitone steps. |
| **Pitch — Fine** | Fine-tune in cents. |
| **Tone — Low** | Low shelf, ±12 dB at 250 Hz. |
| **Tone — High** | High shelf, ±12 dB at 4 kHz. |
| **Sample Start** | Trim where playback begins. |
| **Sample End** | Trim where playback ends. |
| **Amplitude — Volume** | Overall output level. |
| **Amplitude — Pan** | Position the sound left or right in the stereo field. |

Pitch is **global** — it applies to every sample the same way.

---

## 7. Per-note randomization

Below each knob is a **dual slider** with two thumbs:

- The **left thumb** sets how far the value can drop (negative range).
- The **right thumb** sets how far it can rise (positive range).

On every note, Robin Control Lite picks a fresh random value within that range.
Leave both thumbs at the center for no variation; widen them for more movement.

This is the heart of the plugin: a little randomization on pitch, tone, or start
makes footsteps, foley, and percussion sound natural instead of looped.

---

## 8. Help

- **Email:** hello@conduitdsp.com
- **Issues:** report bugs with your DAW, OS, and steps to reproduce.

---

*Robin Control Lite is free. VST is a trademark of Steinberg Media Technologies
GmbH. Audio Units is a trademark of Apple Inc.*
