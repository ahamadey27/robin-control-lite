# Robin Control Premium — Sync Spec from Lite

**Audience:** the Claude Code agent working in the `round-robin-premium` repo (`NewProject/` subfolder). This document tells you what changed in the Lite repo that should propagate to Premium, and — equally important — what should NOT be copied because it's reserved for Pro features that Lite intentionally strips.

**Lite repo on disk:** `/Users/alex/Documents/Github/robin-control-redesign` (folder name predates the rename — VSCode workspace paths kept as-is). Remote: `https://github.com/ahamadey27/robin-control-lite` (private).

**Premium repo:** `https://github.com/ahamadey27/round-robin-premium` (`NewProject/`).

If you have local access to the Lite repo, prefer `git show <sha> -- <path>` for exact diffs over re-deriving from this document. Commit SHAs are referenced inline below.

---

## How to use this spec

1. **Identity first.** Don't blindly copy strings — Pro keeps its own product name, bundle ID, plugin codes, EULA, and license. See §1.
2. **Pro-only code stays active.** Lite has commented-out `ThreeBandEQ`, `TransientShaper`, `juce::ADSR`, paired-key infrastructure, and many randomization parameters. Pro must keep these alive. See §2.
3. **Apply backend fixes wholesale.** §3 — these are correctness/perf wins that aren't Lite-specific.
4. **Audio formats + UX + About + bottom logo are largely portable.** §4–§7.
5. **UI redesign is the largest body of work.** §8 — port aesthetic + LookAndFeel, but keep Pro-only controls visible (envelope, 3-band EQ, transient shaper sections that Lite hides).
6. **Verify per §9.**

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

## 2. Pro-only code Lite strips — DO NOT also strip

Lite's source has many sections marked `// COMMENTED FOR LITE — ACTIVE IN PREMIUM`. Pro **must keep these active**. Inventory:

- **`ThreeBandEQ`** (`Source/DSP/ThreeBandEQ.{h,cpp}`) — full 3-band EQ. Lite declares the member but never `prepareToPlay`s or processes it. Pro should call `threeBandEQ.prepareToPlay(sr, blockSize)`, `threeBandEQ.updateFilters(...)` per block, and `threeBandEQ.processBlock(buffer)` after the synth.
- **`TransientShaper`** (`Source/DSP/TransientShaper.{h,cpp}`) — Lite same pattern. Pro processes after EQ, before volume.
- **`juce::ADSR` envelope** in `RRVoice` — Lite advances it per sample but doesn't apply the gain. Pro applies envelope to output sample, and the dormant `envelope.noteOn()`/`noteOff()` in startNote/stopNote should drive actual amplitude.
- **APVTS parameters Lite hides**: `envAttack`, `envDecay`, `lowGain`/`lowFreq`, `midGain`/`midFreq`, `highGain`/`highFreq`, `transientAttack`, `transientDecay`, plus all their `*RndNeg`/`*RndPos` partners. Lite's `createParameterLayout()` skips them. Pro's must include them — that's why `ParameterIDs::totalParameters = 48` exists (Pro count).
- **Smoothers** for those: `smoothedEnvAttack`, `smoothedEnvDecay`, `smoothedTransientAttack`, `smoothedTransientDecay` — declared in `PluginProcessor.h` but unused in Lite.
- **`rndPtrs.*` for those parameters** in `RRVoice::setRandomizationReferences()` — Lite comments out the assignments; Pro keeps them.
- **Per-note randomization** in `RRVoice::startNote()` for envelope/EQ/transient — Lite comments those blocks out; Pro keeps them.
- **Tone control + 3-band EQ override path** in `processBlock` — Lite has tone-only override of `randomizedToneLow/High`; Pro additionally overrides `randomizedLowGain`/`Freq`, etc.
- **`auditionGetters`**: `getRandomizedLowGain()`, `getRandomizedLowFreq()`, `getRandomizedMidGain()`, etc. on `RRVoice` — Lite comments them out.
- **Paired-key MIDI mapping** (`MidiMapper::NUM_KEY_PAIRS = 10`, `RRSound::keyPairIndex`, `RRSound::setKeyPairIndex`) — Lite ships unpitched / any-key-triggers. Pro likely uses paired-key per its product spec; keep the infrastructure wired.

When porting any code from Lite, **search the file for `COMMENTED FOR LITE` markers and uncomment the Pro side**, OR simply reject Lite's deletion of those blocks during the diff.

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

- **Audio/message-thread safety contract** — copy verbatim from Lite's CLAUDE.md "Key Design Decisions" section. The pattern (`getCallbackLock()`, `SampleLoader` taking `const juce::CriticalSection&`) is identical for Pro.
- **`prepareToPlay` rebuild rule** — same; copy.
- **Sample Start asymmetric edge case** — same; copy. Pro inherits the same trim semantics.
- **ToneControl cache** — same; copy. Add a parallel paragraph for `ThreeBandEQ` if Pro applies the same caching pattern there.
- **MP3 build gate** — same; copy.
- **"Load Samples is additive"** — same UX; copy.

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

## 9. Verification

After porting, smoke-test on Pro:

1. **Build clean** — `cmake -B build && cmake --build build`. Verify all formats build (VST3, AU, Standalone, AAX if Pro ships AAX).
2. **Identity** — open in DAW, confirm Pro's product name appears, not Lite's. Bundle ID, plugin code, manufacturer code distinct from Lite.
3. **Backend correctness:**
   - Load 5 samples, hammer trigger ~10Hz, drag-reorder a slot. No crash, no glitch, no UI desync (race fix from §3a).
   - Open at 44.1k, switch DAW to 96k. All loaded samples replay fully without truncation (§3b).
   - Mixed-duration pool (2 long MP3s + 5 short WAVs), set Random Algorithm to 15/16/17. All 7 samples audible, not just MP3s (§3c).
4. **CPU** — idle CPU should be lower than baseline (tone cache D); panning a long sample should be flat across pan positions (E).
5. **Format** — drop in `.mp3`, confirm load + playback (§4).
6. **UX** — Load Samples button appends instead of replaces (§5a). `?` toggles About (§5b).
7. **Pro-only sections** — verify ENVELOPE, EQ, TRANSIENT (or whatever Pro exposes) still work after porting Lite's APVTS-load consolidation. The randomization for those parameters should fire on note-on. Pro is monophonic? polyphonic? — verify per-voice behavior matches Pro's design.
8. **Visual** — section labels show the four-layer gradient treatment. Bottom-right logo present. About popup centered.
9. **Preset compatibility** — save a preset, close, reopen, load. All Pro's parameters and sample paths restore correctly (Lite param-ID-frozen rule applies equally to Pro).

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

`git -C /Users/alex/Documents/Github/robin-control-redesign show <sha> -- <path>` will show exact diffs.

---

**End of spec.** Questions / ambiguities should go back to the user before applying anything that conflicts with Pro's existing direction.
