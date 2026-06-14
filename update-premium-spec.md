# Robin Control Premium — Sync Spec from Lite

**Audience:** the Claude Code agent working in the `round-robin-premium` repo (`NewProject/` subfolder). This document tells you what changed in the Lite repo that should propagate to Premium, and — equally important — what should NOT be copied because it's reserved for Pro features that Lite intentionally strips.

**Lite repo on disk:** `/Users/alex/Documents/Github/robin-control-redesign` (folder name predates the rename — VSCode workspace paths kept as-is). Remote: `https://github.com/ahamadey27/robin-control-lite` (private).

**Premium repo:** `https://github.com/ahamadey27/round-robin-premium` (`NewProject/`).

If you have local access to the Lite repo, prefer `git show <sha> -- <path>` for exact diffs over re-deriving from this document. Commit SHAs are referenced inline below.

**Last updated:** 2026-05-24. Reflects Lite state through commit `26fb902` (post-installer scaffolding, post Path-B decision). Older sections call out their own snapshot date where relevant.

---

## How to use this spec

1. **Identity first.** Don't blindly copy strings — Pro keeps its own product name, bundle ID, plugin codes, EULA, and license. See §1.
2. **Pro-only code is assumed already in place.** Lite strips `ThreeBandEQ`, `TransientShaper`, `juce::ADSR`, paired-key wiring, and the Pro-only randomization params. This spec assumes Pro has already kept those alive — §2 is now a short verification checklist, not a porting guide.
3. **Apply backend fixes wholesale.** §3 — these are correctness/perf wins that aren't Lite-specific.
4. **Audio formats + UX + bottom logo + CLAUDE.md mirror are largely portable.** §4–§7.
5. **UI redesign is the largest body of work.** §8 — port aesthetic + LookAndFeel, keep Pro-only controls visible.
6. **Build/release/installer infrastructure is new since 2026-04-27.** §9 covers CMake reshape (AAX gate, Standalone drop, deployment target), version stamping, Mac `.pkg` builder, `launch.json` switch, and the `Releases/` tree.
7. **Release execution.** §10 covers the Path-A/B sequencing decision tree and the `release-spec.md` playbook pattern Pro should mirror.
8. **Verify per §11.**

> **Caveat:** Lite's UI may keep evolving. Treat §8 as a snapshot of where Lite stood on 2026-04-27, not a frozen contract. If you see Lite has moved further when you read this, prefer Lite's current state over this document's specifics.

---

## 1. Pro identity — KEEP YOURS

Lite identity strings (DO NOT adopt these for Pro):

| Field | Lite value |
|---|---|
| Product name | `Robin Control Lite` |
| Plugin code | `rcll` |
| Manufacturer code | `Cdsp` |
| Bundle ID | `dsp.conduit.RobinControlLite` |
| CMake target | `RobinControlLite` |

Pro should have its own equivalents (`Robin Control` / `rcrp` or similar / same `Cdsp` manufacturer / `dsp.conduit.RobinControl` / `RobinControl`). Keep whatever's already in Premium's `CMakeLists.txt` and `Source/PluginProcessor.cpp::getName()`.

**LICENSE / EULA / Privacy:** Lite has its own `LICENSE` (proprietary, All Rights Reserved), `EULA.md` (effective 2026-04-26), and `Privacy.md`. Pro should have parallel-but-distinct documents. Don't copy verbatim; cross-reference structure only.

**About-window text:** the body copy in `PluginEditor.h::AboutWindow::bodyText` is Lite-specific ("Load up to 20 samples..." etc.). Pro's About should describe Pro's feature set (3-band EQ, transient shaper, ADSR, whatever else). Keep the title format: `"Robin Control"` in cream + `"Pro"` (or whatever marker) in amber.

---

## 2. Pro-only code — assumed already in place

Per the user, Pro's core logic and parameters (ThreeBandEQ, TransientShaper, ADSR envelope, paired-key MIDI mapping, all the hidden APVTS params and their `*RndNeg`/`*RndPos` partners, the `rndPtrs.*` wiring in `RRVoice`, per-note randomization for env/EQ/transient, auditionGetters, the 3-band EQ override path in `processBlock`) is **already ported and active**. This section is retained as a quick checklist if you need to spot-check during the merge:

- `ThreeBandEQ::prepareToPlay` / `updateFilters` / `processBlock` are actually called in Pro's `processBlock` (not dormant).
- `TransientShaper` runs after EQ, before volume.
- `RRVoice` applies the ADSR envelope gain to the output sample, and `envelope.noteOn()/noteOff()` is wired in `startNote`/`stopNote`.
- Pro's `createParameterLayout()` includes the Pro-only parameter IDs and partners; `ParameterIDs::totalParameters` matches Pro's count (Lite is 47, Pro should be 48 or whatever Pro's spec calls for).
- The Pro-only smoothers (`smoothedEnvAttack`, `smoothedEnvDecay`, `smoothedTransientAttack`, `smoothedTransientDecay`) are advanced per block.

When merging Lite changes from this point on, the rule is: if a Lite diff deletes a Pro-active block (e.g. removes an EQ/transient call), reject that part of the diff. Lite-only `// COMMENTED FOR LITE` markers can be ignored on the Pro side.

---

## 3. Backend fixes (apply wholesale)

These are correctness/performance fixes not tied to Lite's feature reduction. Apply directly.

### 3a. Audio/message-thread safety contract — Lite commit `17637da`

**Problem:** `swapSamples`, `insertSample`, `SampleLoader::loadSample`, etc. mutate `sampleSlots[]` from the message thread. The audio thread reads them inside `advanceRoundRobin → RRSound::setFromSlot` (called from `processBlock`). `juce::AudioBuffer<float>::operator=` is a deep copy — concurrent reallocation by the loader corrupts the read.

**Fix:** wrap message-thread mutations in `juce::ScopedLock(getCallbackLock())`. The audio thread holds it implicitly during `processBlock`, so this serializes cleanly without slowing audio.

**Files to change** (paths relative to `NewProject/`):

- `Source/Data/SampleLoader.h` — constructor takes additional `const juce::CriticalSection& callbackLock` param; store as `const juce::CriticalSection& callbackLock` member.
- `Source/Data/SampleLoader.cpp` — init list takes the lock; `loadSample`/`clearSlot`/`setSampleRate`/`updateSynthesiserSounds` wrap mutations in `const juce::ScopedLock sl(callbackLock);`. **Critical:** decode and resample happen OFF-lock (allocate a `SampleSlot pending`, fill it, resample it, THEN take the lock and do `slots[slotIndex] = std::move(pending);`). Same pattern for `setSampleRate`'s loop. This keeps `processBlock` from stalling on big files.
- `Source/PluginProcessor.h` — sampleLoader member init: `SampleLoader sampleLoader{ formatManager, synthesiser, sampleSlots, NUM_SAMPLE_SLOTS, getCallbackLock() };`
- `Source/PluginProcessor.cpp`:
  - `swapSamples`, `insertSample`, `auditionSample`, `resetPlaybackPosition` — wrap entire body in `const juce::ScopedLock sl(getCallbackLock());`. Move `sampleLoader.updateSynthesiserSounds()` OUTSIDE the lock (it locks itself).
  - `setStateInformation` — wrap the slot-restore section: at minimum, the `rebuildLoadedIndices(); reshuffleIndices();` call.

`juce::CriticalSection` is recursive, so re-entering from inner calls (e.g., `clearSlot` when called inside an outer `setStateInformation` lock) is fine.

`getCallbackLock()` returns `const juce::CriticalSection&` in JUCE 8 — your reference and parameter must be `const`-qualified.

### 3b. Sample-rate switch fix — Lite commit `a62b262`

**Problem:** when DAW switches sample rate, `prepareToPlay` is called with the new rate. `sampleLoader.setSampleRate()` resamples all loaded slots. `maxSampleLength` (used by voices to compute trim/start/end percentages) was computed at the OLD rate and stays stale. At 88.2k or 96k from a 44.1k baseline, voices clamp playback to the old sample count and audio truncates.

**Fix:** call `rebuildLoadedIndices()` immediately after `sampleLoader.setSampleRate(...)` in `prepareToPlay`.

```cpp
sampleLoader.setSampleRate(sampleRate);
rebuildLoadedIndices();   // <-- ADD: refreshes maxSampleLength at the new rate
synthesiser.setCurrentPlaybackSampleRate(sampleRate);
```

### 3c. Mixed-duration Sample Start fallback — Lite commit `79fb930`

**Problem:** Sample Start/End percentages reference `maxPoolSampleLength` (the longest loaded sample), then clamp per-voice to `cachedSampleLength`. End-clamp is fine: short samples play fully when End extends past them. But Start-clamp silences short samples — if pool-relative Start > this sample's length, `playbackStartSample` clamps to cachedSampleLength, equal to playbackEndSample, and the voice exits with "nothing to play". This shows up as silent short samples when Random Algorithm tick 15+ adds a `sampleStartRndPos` offset, especially in pools containing one very long sample (e.g., a multi-minute MP3 alongside short WAVs).

**Fix:** in `RRVoice::startNote()`, when pool-relative Start lands past this sample's end, fall back to applying the Start percentage to *this* sample's own length. End uses straight clamp as before.

```cpp
const int refLength = (maxPoolSampleLength > 0) ? maxPoolSampleLength : cachedSampleLength;

int absoluteStart = (int)(randomizedSampleStart / 100.0f * refLength);
int absoluteEnd   = (int)(randomizedSampleEnd   / 100.0f * refLength);

playbackEndSample = juce::jlimit(1, cachedSampleLength, absoluteEnd);

if (absoluteStart >= playbackEndSample)
    playbackStartSample = (int)(randomizedSampleStart / 100.0f * cachedSampleLength);
else
    playbackStartSample = absoluteStart;

playbackStartSample = juce::jlimit(0, playbackEndSample - 1, playbackStartSample);
```

The old `if (playbackEndSample <= playbackStartSample) { early-exit }` guard becomes structurally unreachable after this; you can drop it.

### 3d. CPU optimizations — Lite commit `17637da`

Apply directly. Each is independent and safe.

**ToneControl coefficient cache** (`Source/DSP/ToneControl.{h,cpp}`): cache last-applied `(lowGain_dB, highGain_dB)` as float members initialized to `std::numeric_limits<float>::quiet_NaN()`. In `updateFilters`, after the `jlimit` clamps, early-return if `abs(newLow - lastLow) < 0.001f && abs(newHigh - lastHigh) < 0.001f`. After computing new coefficients, store the new values. **Invalidate the cache in `prepareToPlay`** (set both back to NaN, then call `updateFilters(0,0)` AFTER setting `isPrepared = true`). This is the largest single CPU win when knobs are idle.

**Pro extension:** apply the same pattern to `ThreeBandEQ` and any other per-band coefficient-recomputing processor Pro has active. Same template: cache last gains/freqs/Qs, early-return if unchanged.

**Pan gains cached at note-on** (`Source/Audio/RRvoice.{h,cpp}`): add `cachedPanLeftGain` and `cachedPanRightGain` float members to `RRVoice` (init `0.7071f` each). At the end of `startNote()` (after randomizedPan is set), compute equal-power gains once:

```cpp
const float panAngle = (randomizedPan + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
cachedPanLeftGain  = std::cos(panAngle);
cachedPanRightGain = std::sin(panAngle);
```

Then in `renderNextBlock`, replace the per-sample sin/cos with these cached values:

```cpp
outputBuffer.addSample(0, startSample + i, outputSample * cachedPanLeftGain);
outputBuffer.addSample(1, startSample + i, outputSample * cachedPanRightGain);
```

Removes ~96k transcendentals/sec per playing voice. **Pro caveat:** if Pro has live pan smoothing (Lite doesn't), keep the smoother and don't apply this optimization — only valid when pan is fixed for the voice's life.

**APVTS atomic load consolidation** (`PluginProcessor::processBlock`): each `apvts.getRawParameterValue(...)->load()` is two atomic ops. Lite was reading `semitone` and `fineTune` 2–3× per block. Read each parameter into a local at top of `processBlock`, reuse. Pull voice-broadcast values from the smoother (`smoothedSemitone.getCurrentValue()`) instead of re-loading the same atomic.

**`juce::ScopedNoDenormals` in `RRVoice::renderNextBlock`** (defense in depth — `processBlock` already declares it but defining it in the voice render too costs nothing on x86_64 and protects against future host scenarios).

### 3e. Unconditional declick fade-in — Lite commit `17637da`

**Problem:** Lite is monophonic; new note-on triggers JUCE Synthesiser to call `stopNote(0, true)` followed by `startNote` on the same voice. With ADSR dormant, stopNote(true) is a no-op and the new sample begins at frame 0 with no fade. Block-boundary amplitude discontinuity → audible click on rapid retriggers. Lite previously gated the 3ms fade-in on `sampleStart > 0%`.

**Fix:** drop the `if (randomizedSampleStart > 0.0f && ...)` guard at the fade-in branch in `RRVoice::renderNextBlock`. Apply the 3ms micro-fade-in on every note start.

```cpp
if (fadeSamples > 0)
{
    int pos = (int)sourceSamplePosition;

    // Unconditional fade-in declicks retriggers (was guarded by sampleStart > 0%)
    if (pos < playbackStartSample + fadeSamples)
    {
        float linear = (float)(pos - playbackStartSample) / (float)fadeSamples;
        outputSample *= std::pow(juce::jlimit(0.0f, 1.0f, linear), 0.1f);
    }

    // Fade-out at end stays guarded — only relevant when trimming
    if (randomizedSampleEnd < 100.0f && pos >= playbackEndSample - fadeSamples)
    {
        float linear = (float)(playbackEndSample - pos) / (float)fadeSamples;
        outputSample *= std::pow(juce::jlimit(0.0f, 1.0f, linear), 0.1f);
    }
}
```

**Pro caveat:** if Pro is polyphonic, monophonic-stealing clicks aren't the same risk. The fade-in is still good practice and harmless for one-shot voices. Keep it. If Pro uses ADSR for amplitude (not just dormant), the envelope's attack provides its own declick and you can leave the fade-in only for the trim case — your call.

---

## 4. Audio format support — MP3 — Lite commit `79fb930`

JUCE 8 does NOT register MP3 in `registerBasicFormats()`. To accept MP3:

1. **`CMakeLists.txt`** — add `JUCE_USE_MP3AUDIOFORMAT=1` to the public compile definitions:
   ```cmake
   target_compile_definitions(<TargetName>
       PUBLIC
           JUCE_WEB_BROWSER=0
           JUCE_USE_CURL=0
           JUCE_VST3_CAN_REPLACE_VST2=0
           JUCE_USE_MP3AUDIOFORMAT=1
       PRIVATE
           JUCE_STRICT_REFCOUNTEDPOINTER=1
   )
   ```
   You'll need to re-run `cmake -B build` after this; `cmake --build build` alone won't pick up the new define.

2. **`PluginProcessor.cpp` constructor**, after `formatManager.registerBasicFormats()`:
   ```cpp
   formatManager.registerFormat(new juce::MP3AudioFormat(), false);
   ```

3. **`SampleLoader.cpp::loadSample`** — add `".mp3"` to the `supportedFormats` array; update the user-facing error message.

4. **File-chooser filters in `PluginEditor.cpp`** — three places (Load Samples / Add More / Replace single slot): change `"*.wav;*.aif;*.aiff;*.flac;*.ogg"` to `"*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3"`.

JUCE's MP3 implementation is decoder-only and ships royalty-free; safe for distribution.

---

## 5. UX behavior changes

### 5a. "Load Samples" is additive, not destructive — Lite commit `79fb930`

**Before:** clicking the header `Load Samples` button cleared the entire pool and replaced with the chosen files.

**After:** Load Samples appends to the next empty slot — same code path as the in-pool "click to add (x) samples" placeholder. Users clear with the Clear button to start fresh.

In `PluginEditor.cpp`:
- Wire `sampleManagerPanel.onLoadSamplesClicked = [this]() { addMoreSamples(); };` (was calling a separate `loadSamplesFromFiles()`).
- Delete `loadSamplesFromFiles()` and its declaration in `PluginEditor.h`.

User explicitly chose this: hitting Load to add one more sample shouldn't nuke the pool.

### 5b. About popup — toggle on `?` button (recently added)

The `?` button now toggles open/close. Click while open → close (in addition to the existing Close button).

```cpp
aboutButton.onClick = [this]
{
    if (aboutWindow.isVisible())
    {
        aboutWindow.setVisible(false);
    }
    else
    {
        aboutWindow.setTopLeftPosition((getWidth()  - aboutWindow.getWidth())  / 2,
                                       (getHeight() - aboutWindow.getHeight()) / 2);
        addAndMakeVisible(aboutWindow);
        aboutWindow.toFront(true);
    }
    repaint();
};
```

The window centering uses **actual** size (`aboutWindow.getWidth()/getHeight()`), not hardcoded offsets. Important: the previous version used `getHeight() / 2 - 100` which assumed window height 200; that's why it was off-center.

The window's body height is measured via `juce::TextLayout` in the constructor so the gap above the close button matches the gap below the title (both 12px). See `PluginEditor.h::AboutWindow` for the implementation — uses `static constexpr int marginTop/titleH/gap/buttonH/marginBottom` and computes total height from measured `bodyHeight`.

### 5c. Version stamp in About window — Lite commit `6542f83`

The About window paints `"v" + JucePlugin_VersionString` in its bottom-right corner, dim warm gray (`0xff7a7468`), 9pt — same font family as the body. It sits at `(getWidth() - 50, getHeight() - 14, 40, 10)`, right-justified. Important: this draws **outside** the measured-text layout, so adding it doesn't perturb the title/body/gap math.

`JucePlugin_VersionString` is emitted by `juce_add_plugin(VERSION ...)`. See §9d for the CMake side (Pro must set `VERSION` in its `juce_add_plugin` call or this define won't exist and the build will fail).

```cpp
// Inside AboutWindow::paint, after the body text is drawn:
g.setFont(juce::Font(juce::FontOptions(9.f)));
g.setColour(juce::Colour(0xff7a7468));
g.drawText(juce::String("v") + JucePlugin_VersionString,
           getWidth() - 50, getHeight() - 14, 40, 10,
           juce::Justification::right);
```

### 5d. `getProgramName(0)` returns `"Default"` — Lite commit `6542f83`

In `PluginProcessor.cpp`, `getProgramName(int index)` now returns `"Default"` instead of an empty string. Silences a pluginval warning. No functional effect for AU (which reports `AUPreset.presetNumber=-1` regardless until factory presets are registered).

```cpp
const juce::String NewProjectAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return "Default";
}
```

---

## 6. Bottom-of-window logo

Lite paints `LOGOS/FULLLOGO-email-white.png` (binary-baked via `juce_add_binary_data` as `BinaryData::FULLLOGOemailwhite_png`) into a footer strip inside `PluginEditor::paint()`:

```cpp
constexpr int footerH = 22;
constexpr int logoH   = 16;   // ~10% smaller; fits 22px footer with 3px top/bottom padding
const int logoW = juce::roundToInt(logoH * (float)logo.getWidth()
                                          / (float)logo.getHeight());
const int logoX = getWidth()  - logoW - 14;     // right-aligned with 14px right margin
const int logoY = getHeight() - footerH + (footerH - logoH) / 2;

g.drawImage(logo, juce::Rectangle<float>((float)logoX, (float)logoY,
                                         (float)logoW, (float)logoH),
            juce::RectanglePlacement::centred, false);
```

Logo image is loaded once via static `[]` lambda (`ImageCache::getFromMemory(BinaryData::..., ...)`). It's the dark/email variant — keyed against the warm-cream footer at runtime; do NOT use the "clear" web/marketing variant for the binary.

`CMakeLists.txt` registers the binary asset:
```cmake
juce_add_binary_data(<TargetName>_BinaryData
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../LOGOS/FULLLOGO-email-white.png
        ${CMAKE_CURRENT_SOURCE_DIR}/../LOGOS/FULLLOGO-standard-clear.png
)
```

Pro should drop its own logo in `LOGOS/` and reference its `BinaryData::*` symbol.

---

## 7. CLAUDE.md — mirror the new rules

Pro's `CLAUDE.md` (if it exists) should mirror these load-bearing rules, adapted for Pro's feature set:

- **Audio/message-thread safety contract** (§3a) — copy verbatim from Lite's CLAUDE.md "Key Design Decisions" section. The pattern (`getCallbackLock()`, `SampleLoader` taking `const juce::CriticalSection&`) is identical for Pro.
- **`prepareToPlay` rebuild rule** (§3b) — same; copy.
- **Sample Start asymmetric edge case** (§3c) — same; copy. Pro inherits the same trim semantics.
- **ToneControl cache** (§3d) — same; copy. Add a parallel paragraph for `ThreeBandEQ` if Pro applies the same caching pattern there.
- **MP3 build gate** (§4) — same; copy.
- **"Load Samples is additive"** (§5a) — same UX; copy.
- **Version is single-source-of-truth in CMakeLists.txt** (§9d) — the `juce_add_plugin(VERSION ...)` line + the `project(... VERSION)` line MUST match; the installer script greps the latter.
- **macOS deployment target = 11.0** (§9a) — sync between `CMakeLists.txt`, `distribution.xml`, and any `cmake -D...` overrides.
- **AAX SDK gate** (§9b) — document the env path (`~/SDKs/aax-sdk-2-9-0` on Mac, `%USERPROFILE%\SDKs\aax-sdk-2-9-0` on Windows) and the `JUCE_AAX_SDK_PATH` override.
- **Standalone status** (§9c) — if Pro keeps Standalone, document the post-build copy hook; if dropped, document the removal and the attach-debug `launch.json` switch (§9f).

Lite's CLAUDE.md is at the repo root: `/Users/alex/Documents/Github/robin-control-redesign/CLAUDE.md`. Read it directly for the exact wording — Pro's prose can match.

---

## 8. UI redesign — warm-cream / Akai S612-inspired aesthetic

This is the biggest body of work. Pro keeps Pro-only sections visible (envelope controls, 3-band EQ panel, transient shaper) but adopts the same visual language.

### 8a. Color palette (`Source/UI/RRLookAndFeel.h::namespace RRColors`)

Port the entire `RRColors` namespace verbatim. Concrete values:

```cpp
// Surfaces
background    = 0xff8a8478   // warm gray outer panel / footer field
backgroundLo  = 0xff7a7468   // darker warm gray (brush grain)
headerBg      = 0xffc7c2b2   // cream header strip
sectionBg     = 0xffe4dfcd   // brighter cream wells (label-readable surface)
sectionBgDark = 0xffcdc7b4   // slightly darker cream (gradient bottom)
sectionBorder = 0xff2a2418   // dark warm bevel for depth

// Knobs
knobBody  = 0xff8a8478       // warm gray cap
knobRim   = 0xff1f1710       // warm near-black rim (umber-tinted for cream panel)
knobTrack = 0xff1a1713

// Text
valueText   = 0xffe6f0ff     // LED readout (pale blue-white)
companyText = 0xff4a4030     // dark warm ink on cream
screenPrint = 0xffe6e1d4     // warm white silk-screen label
panelInk    = 0xff2a2418     // dark ink text on cream

// Accents
s612Red    = 0xffd83030      // record / panic
s612RedDim = 0xff8a3030      // trigger variant
s612Teal   = 0xff2fa9a1      // accents (NOT for Save/Load)
amber      = 0xffffb84a      // accents
ledGreen   = 0xff3de070      // digital LED green (meter)
liteShade  = 0xffd83030      // "Lite" subtitle tint — Pro renames this to a Pro accent

// LCD field
lcdBg        = 0xff1e4eb0    // deep blue
lcdBgDark    = 0xff163a8c    // inner shadow
lcdText      = 0xffe6f0ff
lcdTextDim   = 0xff8cb0e8
lcdHighlight = 0xffb0d0ff
lcdRed       = 0xffff9080

// Section accent colors (used for knob rings, randomization arcs, and section headers)
pitchCol = 0xff9c2a2e        // deeper red
ampCol   = 0xff00704f        // deeper S612 green
envCol   = 0xffb88030        // amber — Pro USES this (ENVELOPE section); Lite reserves it
transCol = 0xff8858b0        // purple — Pro USES this (TRANSIENT); Lite reserves
toneCol  = (check Lite RRLookAndFeel.h for current value)
trimCol  = (check Lite RRLookAndFeel.h)
```

Pro should add or unhide section-color entries for any sections Lite doesn't display.

### 8b. LookAndFeel classes (`Source/UI/RRLookAndFeel.h/cpp`)

Custom `LookAndFeel_V4` subclasses Lite uses. Port them all and use them on the corresponding controls:

- **`RRKnobLAF`** — rotary knob look. Warm-gray cap, near-black rim, colored ring around the active value arc. Section color (pitch/amp/tone/trim) passed in or set via `setColour`.
- **`RRNegSliderLAF` / `RRPosSliderLAF`** — used by the hidden APVTS-bound sliders feeding the dual-thumb randomization bars (Lite uses these as data carriers; visuals come from `DualThumbRndSlider`).
- **`RRToggleLAF`** — toggle look (e.g. playback mode Series/Random).
- **`RRButtonLAF`** — button look used by Trigger / Panic / About / Save / Load.

Read Lite's `RRLookAndFeel.cpp` for the actual draw code. The aesthetic intent: **silk-screen labels on a cream panel, S612-style colored ring around knob value, near-black rim, no skeuomorphic gradients**.

### 8c. Section-label gradient treatment — `drawSectionTitle` lambda in `PluginEditor.cpp::paint`

Every section header — **SAMPLE POOL**, **RANDOM ALGORITHM**, **AMPLITUDE**, **TONE**, **PITCH**, **SAMPLE START/END** (and Pro adds **ENVELOPE**, **EQ**, **TRANSIENT** etc.) — uses a unified four-layer treatment. **Don't simplify to a single fill; user explicitly approved this look.**

1. Cream `0xffece5d4` 1px drop-shadow underneath at alpha 0.55
2. Dark `0xff0a0806` 1px outline stroke
3. Vertical gradient fill on the path: `accent.brighter(0.15f)` at top → `accent.darker(0.10f)` at bottom (S612 button-strip technique)
4. Faint white sheen along glyph tops: 0.8px tall rect at 25% alpha, clipped to the text path

`drawSectionTitle(text, x, y, w, h, accentColour)` — see Lite's `PluginEditor.cpp` ~line 666 for exact body. A sibling implementation lives in `SampleManagerPanel.cpp::paint` for the SAMPLE POOL header (it uses the same technique because the panel paints itself separately).

**SAMPLE POOL header position is hard-locked at `y=3.0f` in `SampleManagerPanel.cpp`.** User approved this exact position after several iterations — do not move.

### 8d. DualThumbRndSlider (`Source/UI/DualThumbRndSlider.h`, header-only)

Below each randomization-capable knob: a horizontal bar with two thumbs (neg on left, pos on right), 4px gap at 0%. Reads/writes from two hidden APVTS-bound `juce::Slider`s — the slider component itself doesn't own the data.

Drawn over the knob's bottom edge. `paintOverChildren()` in `PluginEditor.cpp` adds thin arc outlines around the knob (2px stroke at 70% alpha, CCW for neg / CW for pos) keyed to the same neg/pos values.

Pro should use this for every randomization-capable parameter, including the Pro-only ones (envelope attack/decay, EQ low/mid/high gain & freq, transient attack/decay).

### 8e. SampleManagerPanel (`Source/UI/SampleManagerPanel.{h,cpp}`)

Left-side panel hosting the sample pool. Features:

- Sample list (numbered slots, filename in each loaded slot, "click to add" placeholder in empties).
- Per-slot: load/replace/audition/delete buttons (some via context menu).
- Drag-to-reorder (calls `processor.swapSamples()` for adjacent or `processor.insertSample()` for non-adjacent).
- Playback mode toggle (Series / Random) at the top.
- "Load Samples" button at the top — additive (see §5a).
- "Clear All" / "Reset Pool" controls.
- Header reads SAMPLE POOL with the gradient-label treatment, locked at y=3.

Read Lite's source for layout specifics; copy the structure and let Pro extend with any additional per-slot actions Pro needs.

### 8f. Header buttons (right-to-left)

Lite header layout, right-to-left: **About `?`, Panic `!`, Trigger, Save, Load**. Save/Load are user presets (green muted); Trigger is muted red; Panic is bright red; About is neutral warm gray.

Pro keeps this layout. If Pro adds a header button (e.g. polyphony toggle), insert in a logical place but don't reorder existing five.

### 8g. Canvas size

Lite is `1400 × 400`. Pro likely needs taller/wider for the additional EQ + transient + envelope sections. Whatever Pro's existing canvas size is, keep it; just port the visual language.

### 8h. Level meter

Lite has an output level meter (linear gain, fast attack / slow release envelope). Implementation: `processBlock` writes `outputPeakLevel.store(peak, std::memory_order_relaxed)` at the end; editor's `juce::Timer` callback pulls the value and applies a fast-attack/slow-release envelope to `displayedLevel`, then repaints just the meter rect. Color: `RRColors::ledGreen` with red overload tip.

Worth porting if Pro doesn't already have one.

---

## 9. Build & release infrastructure (post-2026-04-27)

Everything in this section landed after the original spec was written. It's mostly CMake reshape + Mac installer scaffolding + dev ergonomics, all driven by getting Lite to a shippable signed/notarized `.pkg`. Pro will hit the same checklist.

### 9a. macOS deployment target → 11.0 — Lite commit `acfd637`

`CMakeLists.txt` line 2 was lowered from `13.0` to `11.0`:

```cmake
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
```

Rationale (from `spec.md` §7): 11.0 (Big Sur) covers Apple Silicon's first OS and Intel users on older machines. 13.0 was too aggressive for a free plugin's target audience. The `distribution.xml` installer also gates on `<os-version min="11.0"/>` — keep both in sync.

### 9b. AAX SDK opt-in gate — Lite commit `acfd637`

`CMakeLists.txt` now auto-detects the AAX SDK and adds AAX to `FORMATS` only when present, so fresh clones / Windows boxes without the SDK still configure cleanly:

```cmake
set(JUCE_AAX_SDK_PATH "$ENV{HOME}/SDKs/aax-sdk-2-9-0" CACHE PATH "Path to AAX SDK root")

if(EXISTS "${JUCE_AAX_SDK_PATH}/Interfaces/AAX.h")
    message(STATUS "AAX SDK found at ${JUCE_AAX_SDK_PATH} — enabling AAX format")
    juce_set_aax_sdk_path("${JUCE_AAX_SDK_PATH}")
    set(PLUGIN_FORMATS VST3 AU AAX)
else()
    message(STATUS "AAX SDK not found at ${JUCE_AAX_SDK_PATH} — building VST3/AU only")
    set(PLUGIN_FORMATS VST3 AU)
endif()

juce_add_plugin(<TargetName>
    # ...
    FORMATS ${PLUGIN_FORMATS}
)
```

Override per invocation: `cmake -DJUCE_AAX_SDK_PATH=/path/to/aax-sdk -B build`. On Windows, `%USERPROFILE%\SDKs\aax-sdk-2-9-0` is the default — same env-relative pattern.

AAX has no `*_COPY_DIR` equivalent in JUCE; the eval `.aaxplugin` lands in `build/<Target>_artefacts/<Config>/AAX/` and you copy it manually to `/Library/Application Support/Avid/Audio/Plug-Ins/` for Pro Tools Developer testing.

### 9c. Standalone format removed from v1.0 — Lite commit `acfd637` + `84345f0`

Lite dropped Standalone for v1.0. Rationale: nobody asked for it, it doubled the signing/notarization surface area, and the `.app` post-build copy hook was an ongoing maintenance burden. `FORMATS` is now `VST3 AU` (plus AAX when gated), and the manual `add_custom_command(...)` block that copied the Standalone `.app` to `/Applications` was deleted from `CMakeLists.txt`.

If Pro currently ships Standalone, **don't auto-follow** — Pro's audience may differ. But if you do drop it, the cleanup is:
- Remove `Standalone` from the `FORMATS` line (or the `PLUGIN_FORMATS` set above).
- Delete the `if(APPLE) add_custom_command(TARGET <Target>_Standalone POST_BUILD ...)` block.
- Update `installer/build-installer.sh`, `installer/distribution.xml`, `installer/uninstall.sh` to drop the Standalone component pkg and the `/Applications` rm.
- Update `.vscode/launch.json` per §9f.
- Update README install/quick-start sections.

### 9d. JUCE plugin `VERSION` binding — Lite commit `6542f83`

`juce_add_plugin` now takes a `VERSION` argument:

```cmake
juce_add_plugin(<TargetName>
    VERSION 1.0.0
    COMPANY_NAME "..."
    # ...
)
```

This is what populates `JucePlugin_VersionString` (used by the About window version stamp — see §5c) AND what the installer build script greps out of `CMakeLists.txt`:

```bash
VERSION="$(grep -E '^project\(<TargetName> VERSION' "$CMAKE_FILE" \
            | sed -E 's/.*VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/')"
```

`CMakeLists.txt` is the **single source of truth for version**. The `project(... VERSION x.y.z)` line at the top + the `juce_add_plugin(VERSION x.y.z)` line MUST match. Bump both together when cutting releases.

### 9e. Mac installer scaffolding — `installer/` — Lite commits `6542f83` + `84345f0`

Lite ships a productbuild-based `.pkg` builder under `installer/`. Pro should adopt the same pattern; the only changes are Pro's identity strings + artefact paths. Three files:

**`installer/build-installer.sh`** — main entry point. Inputs are the Release artefacts at `NewProject/build-release/<Target>_artefacts/Release/{VST3,AU}/`; output is `Releases/Installers/<Product> <Version>.pkg`. Pipeline:

1. Parse `VERSION` from `NewProject/CMakeLists.txt` (single source of truth).
2. `pkgbuild` one component `.pkg` per format with stable identifier (e.g. `dsp.conduit.RobinControlLite.vst3`, `.au`), `--install-location` to the correct system plugin folder.
3. Stage `EULA.md` as the installer GUI's `license.txt`.
4. `sed` substitute `@VERSION@` in `distribution.xml` → staging dir.
5. `productbuild --distribution ... --package-path ... --resources ... [--sign $INSTALLER_SIGN] <output>.pkg`.

Optional signing: `INSTALLER_SIGN="Developer ID Installer: CONDUIT DSP LLC (TEAMID)"` env var threads through to `productbuild --sign`. Default is unsigned (for local smoke-testing via `sudo installer -pkg ... -target /`).

**`installer/distribution.xml`** — distribution definition. Key elements:

```xml
<options customize="allow" require-scripts="false" hostArchitectures="x86_64,arm64" />
<volume-check>
    <allowed-os-versions>
        <os-version min="11.0" />
    </allowed-os-versions>
</volume-check>
<choices-outline>
    <line choice="vst3" />
    <line choice="au" />
</choices-outline>
<choice id="vst3" title="VST3" description="..."><pkg-ref id="dsp.conduit.RobinControlLite.vst3" /></choice>
<choice id="au"   title="Audio Unit (AU)" description="..."><pkg-ref id="dsp.conduit.RobinControlLite.au" /></choice>
<pkg-ref id="dsp.conduit.RobinControlLite.vst3" version="@VERSION@" auth="Root">RobinControlLite-VST3.pkg</pkg-ref>
<pkg-ref id="dsp.conduit.RobinControlLite.au"   version="@VERSION@" auth="Root">RobinControlLite-AU.pkg</pkg-ref>
```

For Pro: rename all `dsp.conduit.RobinControlLite.*` identifiers to Pro's reverse-DNS, update `<title>` and `<organization>`, swap artefact filenames. Keep `customize="allow"` so users can deselect formats. Keep `@VERSION@` literally — `build-installer.sh` substitutes it at build time.

**`installer/uninstall.sh`** — confirms with the user, then `sudo rm -rf` the system bundles, `rm -rf` the user-local dev copies in `~/Library/Audio/Plug-Ins/{VST3,Components}`, and `pkgutil --forget` the component identifiers so a re-install gets fresh receipts. Pro just needs the path/identifier strings updated.

### 9f. `.vscode/launch.json` → attach-to-process — Lite commit `acfd637`

Since Standalone is gone, there's no `.app` to launch directly. The launch config now attaches lldb to a running host:

```json
{
    "name": "Attach lldb to host (DAW or AudioPluginHost)",
    "type": "cppdbg",
    "request": "attach",
    "program": "/usr/bin/true",
    "processId": "${command:pickProcess}",
    "MIMode": "lldb",
    "preLaunchTask": "cmake build"
}
```

Workflow: `cmake --build build` → open the AU/VST3 in any DAW (Logic, Live, Reaper, JUCE AudioPluginHost) → run this config → pick the host's PID from the picker. If Pro keeps Standalone, keep the old launch config alongside this one; otherwise replace.

### 9g. `Releases/` directory layout

Lite's repo root now has a `Releases/` tree mirroring the distribution layout:

```
Releases/
├── Installers/   # macOS .pkg files (build-installer.sh output)
├── macOS/        # raw signed/notarized bundles before bundling
├── Windows/      # Windows release artifacts (.exe installer when ready)
├── README.html   # rendered README for in-installer or web display
└── build_readme_pdf.py
```

The directory tree is preserved via `.gitkeep` files (the contents themselves are gitignored — `Releases/Installers/*.pkg` etc.). Pro should mirror the structure.

**`Releases/build_readme_pdf.py`** — small Python script (uses the `markdown` package) that renders `README.md` to print-friendly HTML at `Releases/README.html`. Open in a browser → Export/Print to PDF. Embedded CSS uses Letter paper + 0.75in margins + a cream/brown palette matching the plugin. Worth porting verbatim and updating the CSS to match Pro's brand colors.

### 9h. README updates worth mirroring — Lite commit `eca4cb0`

The user pass on README that lands with Lite v1.0:

- Drop all Standalone references (install paths, badges, "Drop a `.wav` onto any slot" language).
- Quick Start step 1 now reads: "Click **Load Samples** in the header (or the 'click to add samples' placeholder in the empty pool) and pick up to 20 audio files" — matches the additive UX from §5a.
- Drag-drop language is gone (Lite doesn't support drag-drop in v1.0; if Pro does, keep the drag-drop bullet but list it secondary to the file picker).
- New "Tip: audition samples before loading" subsection points users at the native file picker's preview (macOS column view + Space for QuickLook; Windows preview pane). This is a low-cost UX win — users discover they can audition without committing pool slots.
- Audio formats list now includes `.mp3`.

---

## 10. Release-spec playbook pattern

Lite ships a `release-spec.md` at the repo root: a **sequential, command-level checklist with `[x]`/`[ ]` checkboxes** covering build config, Apple signing, AAX validation, Pro Tools test, Windows build, distribution, post-ship. It's the live tracker — updated as items are completed, with absolute dates next to each `[x]`. This is distinct from `spec.md` (which is the strategy/identity/scope doc); when they disagree on v1.0 specifics, `release-spec.md` wins.

Pro should adopt the same pattern. Adapt headings to Pro's scope. Highlights worth lifting:

### 10a. Path A vs Path B sequencing — Lite committed Path B 2026-05-09

The AAX commercial signing process (PACE Wraptool + iLok license) requires a request to Avid (`audiosdk@avid.com`); lead time is **days to weeks** and unpredictable. Two sequencing strategies:

- **Path A — One-shot ship.** Tag `v1.0.0` only after all three formats (VST3/AU/AAX) on both platforms (Mac/Win) are signed. Preferred if Avid grants in time.
- **Path B — Staged ship.** Tag `v1.0.0` with VST3 (Mac+Win) + AU (Mac), notarized + Mac-signed, **no AAX**. Ship publicly. When Avid grants, build the AAX, PACE-sign, tag `v1.0.1` adding AAX-only artifacts. README install section gets an "AAX coming soon" line at v1.0, removed at v1.0.1.

**Commit-to-Path-B trigger:** if the date you're otherwise ready to ship is >7 days past Avid's first acknowledgment email and Wraptool/license still hasn't appeared, ship Path B and treat AAX as fast-follow.

Pro will hit the same decision. Send the Avid request on **day 1** of release work, not at the end.

### 10b. Apple signing + notarization sequence

`release-spec.md` §6 has the exact command sequence Pro will need. Outline:

1. **Universal Release build:** `cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0`
2. Verify universal: `file <bundle>/Contents/MacOS/<binary>` should report `Mach-O universal binary with 2 architectures: [x86_64] [arm64]` for each format.
3. **Sign each plugin bundle** with `codesign --force --options runtime --timestamp --sign "$CERT" --deep <bundle>` where `$CERT="Developer ID Application: <COMPANY> (TEAMID)"`. **Don't apply Apple `codesign` to the AAX bundle** — AAX gets PACE-wrapped instead.
4. **Build the .pkg installer:** `INSTALLER_SIGN="Developer ID Installer: ..." installer/build-installer.sh`.
5. **Notarize:** `xcrun notarytool submit <pkg> --keychain-profile <profile> --wait`. Profile is set up once via `xcrun notarytool store-credentials` using the app-specific password generated at appleid.apple.com.
6. **Staple:** `xcrun stapler staple <pkg>` then `xcrun stapler validate <pkg>`.
7. **Gatekeeper test:** `spctl --assess --type install <pkg>` should return `accepted, source=Notarized Developer ID`.

### 10c. Mirror `release-prep-checklist.md` for the lighter-weight version

Lite also keeps a `release-prep-checklist.md` (smaller, more recent). Both files coexist — `release-spec.md` is the deep playbook, `release-prep-checklist.md` is the lighter status tracker.

---

## 11. Verification

After porting, smoke-test on Pro:

1. **Build clean** — `cmake -B build && cmake --build build`. Verify expected formats build (VST3, AU, AAX if Pro ships AAX, Standalone if Pro keeps it). Confirm AAX gate (§9b) reports the expected message based on SDK presence.
2. **Identity** — open in DAW, confirm Pro's product name appears, not Lite's. Bundle ID, plugin code, manufacturer code distinct from Lite.
3. **Backend correctness:**
   - Load 5 samples, hammer trigger ~10Hz, drag-reorder a slot. No crash, no glitch, no UI desync (race fix from §3a).
   - Open at 44.1k, switch DAW to 96k. All loaded samples replay fully without truncation (§3b).
   - Mixed-duration pool (2 long MP3s + 5 short WAVs), set Random Algorithm to 15/16/17. All 7 samples audible, not just MP3s (§3c).
4. **CPU** — idle CPU should be lower than baseline (tone cache §3d); panning a long sample should be flat across pan positions (§3d).
5. **Format** — drop in `.mp3`, confirm load + playback (§4).
6. **UX** — Load Samples button appends instead of replaces (§5a). `?` toggles About (§5b). About window shows `v<version>` in bottom-right (§5c).
7. **Pro-only sections** — verify ENVELOPE, EQ, TRANSIENT (or whatever Pro exposes) still work after merging Lite's APVTS-load consolidation. The randomization for those parameters should fire on note-on. Pro is monophonic? polyphonic? — verify per-voice behavior matches Pro's design.
8. **Visual** — section labels show the four-layer gradient treatment. Bottom-right logo present. About popup centered.
9. **Preset compatibility** — save a preset, close, reopen, load. All Pro's parameters and sample paths restore correctly (Lite param-ID-frozen rule applies equally to Pro).
10. **Version single-source** (§9d) — change `project(... VERSION x.y.z)` and `juce_add_plugin(VERSION x.y.z)` to a fresh version, rebuild, confirm About stamp updates and `installer/build-installer.sh` produces `... x.y.z.pkg`.
11. **macOS deployment target** (§9a) — `otool -l <bundle>/Contents/MacOS/<binary> | grep -A2 LC_BUILD_VERSION` shows `minos 11.0` (or Pro's chosen floor). `distribution.xml`'s `<os-version min="..."/>` matches.
12. **Installer end-to-end** (§9e) — `installer/build-installer.sh` (unsigned) produces a `.pkg` in `Releases/Installers/`. Install it with `sudo installer -pkg ... -target /` on a test Mac; confirm VST3 + AU land in `/Library/Audio/Plug-Ins/{VST3,Components}` and both load in their DAWs. Run `installer/uninstall.sh` and confirm both are removed plus `pkgutil --pkg-info <id>` returns "No receipt".
13. **Signed/notarized install** (§10b) — once Apple certs are set up, repeat #12 with `INSTALLER_SIGN=...` and notarize. `spctl --assess --type install <pkg>` returns `accepted, source=Notarized Developer ID` on a clean Mac.
14. **Attach-debug** (§9f) — if Pro dropped Standalone, confirm `.vscode/launch.json` attach-to-process works against a DAW host. If Pro kept Standalone, confirm the launch-Standalone config still runs.

---

## Quick reference — Lite commit map

If you have the Lite repo cloned:

| Topic | Commit | Files |
|---|---|---|
| Thread safety + CPU refactor | `17637da` | `PluginProcessor.{h,cpp}`, `RRvoice.{h,cpp}`, `ToneControl.{h,cpp}`, `SampleLoader.{h,cpp}` |
| Sample-rate truncation fix | `a62b262` | `PluginProcessor.cpp` |
| MP3 + Load-additive + Sample Start fallback | `79fb930` | `CMakeLists.txt`, `PluginProcessor.cpp`, `PluginEditor.{h,cpp}`, `SampleLoader.cpp`, `RRvoice.cpp` |
| About popup polish | `09348a9` | `PluginEditor.{h,cpp}` |
| Logo darken + About body copy | `7ea87ea` | `PluginEditor.cpp` (and assets) |
| Identity / paperwork (LICENSE/EULA/Privacy/spec) | `19826ff`, `2a2b77d`, `795043c`, `2d9204c` | repo-root docs |
| Mac installer scaffolding + AU getProgramName fix + VERSION bind + About version stamp | `6542f83` | `installer/*`, `CMakeLists.txt`, `PluginProcessor.cpp`, `PluginEditor.h` |
| AAX SDK gate + Standalone drop + deployment target 11.0 + launch.json attach-to-process | `acfd637` | `CMakeLists.txt`, `.vscode/launch.json`, `CLAUDE.md`, `release-spec.md` |
| Installer scripts drop Standalone | `84345f0` | `installer/build-installer.sh`, `installer/distribution.xml`, `installer/uninstall.sh` |
| README rewrite (drop Standalone, additive Load, audition tip) | `eca4cb0` | `README.md` |
| Releases/ tree + README→HTML renderer | `26fb902` | `Releases/build_readme_pdf.py`, `release-spec.md` |
| Release-spec checklist updates (Path B, AAX, signing) | `b6040cd`, `89446fa`, `f9c0e4c`, `26fb902` | `release-spec.md`, `CLAUDE.md` |

`git -C /Users/alex/Documents/Github/robin-control-redesign show <sha> -- <path>` will show exact diffs.

---

**End of spec.** Questions / ambiguities should go back to the user before applying anything that conflicts with Pro's existing direction.
