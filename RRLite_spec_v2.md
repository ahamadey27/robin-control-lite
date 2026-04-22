# Round Robin Lite - Redesign Specification

## Tech Stack: JUCE Framework & C++ | CMake + Visual Studio 2022 (Windows)

This spec covers the redesign of Round Robin Lite from its current completed state (Phases 1-6 of original rrline-spec.md) into the new streamlined product. The plugin retains all existing DSP code internally for future porting to Round Robin Premium, but the UI and exposed parameters are simplified.

**Canvas:** 700x450px (top border may shift up slightly to accommodate header changes)
**Format:** VST3 (Standalone for testing)
**Monophonic:** Single voice, one-shot playback

---

## Current State Summary

The following systems are fully implemented and working:
- 20-slot sample pool with round-robin (Series) and Fisher-Yates shuffle (Random)
- Paired-key MIDI mapping (C0-A2, 10 pairs)
- Global pitch (Semitone + Fine Tune) with per-note asymmetric randomization
- Volume and Pan with per-note asymmetric randomization
- 3-band EQ (low shelf, mid peak, high shelf) with gain/freq randomization
- Transient shaper (attack/decay) with randomization
- ADSR envelope with randomization
- Single "Load Samples" multi-file button
- Series/Random playback toggle
- User preset save/load (.rrpreset)
- Sample persistence via getStateInformation/setStateInformation
- Custom LookAndFeel system (RRKnobLAF, RRNegSliderLAF, RRPosSliderLAF, RRToggleLAF)
- Randomization arc overlay (RndArcOverlay) with blue neg / red pos arcs
- About popup ("?" button)

---

## Phase 1: UI Simplification and Parameter Cleanup

**Goal:** Remove EQ, Transient Shaper, and Envelope sections from the UI and APVTS registration. Keep all DSP code intact in the source files for Premium porting. Rearrange the header and remove the samples-info label.

### Step 1.1: Comment Out APVTS Parameter Registration

- [x] In PluginProcessor.cpp `createParameterLayout()`, comment out (do NOT delete) all parameter registrations for:
  - EQ: lowGain, lowFreq, midGain, midFreq, highGain, highFreq and all 12 associated RndNeg/RndPos parameters
  - Transient: transientAttack, transientDecay and their 4 RndNeg/RndPos parameters
  - Envelope: envAttack, envDecay and their 4 RndNeg/RndPos parameters
- [x] Add a comment block above each group: `// COMMENTED FOR LITE — ACTIVE IN PREMIUM`
- [x] Keep the parameter ID constants in ParametersIDs.h untouched (no changes needed there)

### Step 1.2: Comment Out DSP Processing Calls

- [x] In PluginProcessor.cpp `processBlock()`, comment out the ThreeBandEQ processing call and filter update call
- [x] Comment out the TransientShaper processing call
- [x] Comment out the code that reads EQ/transient/envelope randomized values from the active voice
- [x] Comment out the smoothed parameter reads for EQ, transient, and envelope values
- [x] Keep the ThreeBandEQ and TransientShaper member variables in PluginProcessor.h (just unused)
- [x] Keep `prepareToPlay()` calls for ThreeBandEQ and TransientShaper commented out as well

### Step 1.3: Comment Out Voice Randomization for Removed Parameters

- [x] In RRVoice.cpp `startNote()`, comment out the randomization generation for: EQ bands (low/mid/high gain and freq), transient attack/decay, envelope attack/decay
- [x] In RRVoice.cpp `setRandomizationReferences()`, comment out the pointer caching for all removed parameter IDs
- [x] Keep the member variables (randomizedLowGain, etc.) declared but unused
- [x] In RRVoice.h, comment out the getter functions for removed randomized values (getRandomizedLowGain, etc.)

### Step 1.4: Remove UI Elements for Hidden Parameters

- [x] In PluginEditor.h, comment out all slider/attachment member declarations for: EQ (6 knobs + 12 rnd sliders + 18 attachments), Transient (2 knobs + 4 rnd sliders + 6 attachments), Envelope (2 knobs + 4 rnd sliders + 6 attachments)
- [x] In PluginEditor.cpp constructor, comment out all `addAndMakeVisible()` and slider configuration for the above
- [x] In `resized()`, comment out all `setBounds()` calls for the above components
- [x] In `paint()`, comment out the section headers, borders, and knob labels for EQ, Transient, and Envelope sections
- [x] In `paintOverChildren()`, comment out the `drawRndArcs()` calls for all removed knobs
- [x] Remove the `samplesInfoLabel` member and all references to it (the "No Samples Loaded" / "Samples Loaded: x" text)

### Step 1.5: Restructure Header Layout

- [x] Move "Load Preset" and "Save Preset" buttons to the top-right header bar area (inside the header, not below it)
- [x] Position the "?" (About) button to the right of Save Preset
- [x] Remove the old sub-header row that held Load Samples, samples info, and preset buttons
- [x] The "Load Samples" button moves into the Sample Manager panel (Phase 4)
- [x] The playback mode toggle (Series/Random) moves into the Sample Manager panel (Phase 4)

### Step 1.6: Adjust Remaining Layout

- [x] With only Amplitude (Volume + Pan) and Pitch (Semitone + Fine Tune) remaining as knob sections, reposition them to the right side of the plugin
- [x] Amplitude section: top-right area with Volume and Pan knobs plus their randomization arcs
- [x] Pitch section: below Amplitude or center-right, with Semitone and Fine Tune knobs plus randomization arcs
- [x] The left half of the plugin is reserved for the Sample Manager (Phase 4)
- [x] The center area above Pitch is reserved for the Random Algorithm knob (Phase 6)
- [x] Leave placeholder space for Tone section (Phase 2) and Sample Start/End section (Phase 3)

**Test:**
- [x] Build compiles with 0 errors and 0 warnings related to removed code
- [x] Plugin loads in AudioPluginHost / DAW
- [x] Volume, Pan, Semitone, Fine Tune knobs all function with randomization
- [x] Samples load and play back correctly
- [x] No EQ/Transient/Envelope UI visible
- [x] Header shows plugin name on left, Load Preset / Save Preset / ? on right
- [x] Preset save/load still works (missing parameters gracefully default)

---

## Phase 2: Tone Control (Low/High Shelf EQ)

**Goal:** Add a simplified two-knob tone control using low and high shelf filters at fixed frequencies. This replaces the full 3-band EQ for Lite — giving users quick tonal shaping without complexity. Both knobs have per-note asymmetric randomization.

**Design rationale:** Shelf filters are the industry standard for simple tone controls (Neve, SSL channel strips, DJ mixers). A low shelf broadly boosts/cuts everything below a cutoff, and a high shelf does the same above its cutoff. This is more intuitive and musical than bell/peak filters at fixed frequencies, which only affect a narrow band. Fixed frequencies of 250 Hz (low) and 4 kHz (high) match classic console EQ crossover points — 250 Hz captures body/thump, 4 kHz captures brightness/presence.

### Step 2.1: Register Tone Parameters

- [x] Add parameter IDs to ParametersIDs.h:
  - `toneLow` (float, -12.0 to +12.0 dB, default 0.0)
  - `toneHigh` (float, -12.0 to +12.0 dB, default 0.0)
  - `toneLowRndNeg`, `toneLowRndPos` (float, 0.0 to 1.0, default 0.0)
  - `toneHighRndNeg`, `toneHighRndPos` (float, 0.0 to 1.0, default 0.0)
- [x] Register all 6 parameters in `createParameterLayout()`
- [x] Update `totalParameters` count in ParametersIDs.h
- [x] Display values with one decimal place and "dB" suffix

### Step 2.2: Create ToneControl DSP Class

- [x] Create new files: `Source/DSP/ToneControl.h` and `Source/DSP/ToneControl.cpp`
- [x] Add both files to the `target_sources()` list in `CMakeLists.txt` and re-run CMake to regenerate the VS project
- [ ] ToneControl uses two `juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>` — one for low shelf, one for high shelf (same pattern as ThreeBandEQ but with only two filters)
- [x] Fixed frequencies: low shelf at 250 Hz, high shelf at 4000 Hz
- [x] Q value: 0.707 (Butterworth — standard for shelf filters, gives a smooth flat shelf response without resonant bump)
- [x] `prepareToPlay(double sampleRate, int samplesPerBlock)` — initialize both filters with the processing spec
- [x] `updateFilters(float lowGain_dB, float highGain_dB)` — recalculate coefficients using `juce::dsp::IIR::Coefficients::makeLowShelf()` and `makeHighShelf()` with fixed freq and Q, variable gain
- [x] `processBlock(juce::AudioBuffer<float>& buffer)` — apply low shelf then high shelf in series
- [x] `reset()` — reset both filter states
- [x] Clamp gains to -12.0 to +12.0 dB range inside `updateFilters()` for safety

### Step 2.3: Integrate ToneControl into Processor

- [x] In PluginProcessor.h, add `ToneControl toneControl;` member variable
- [x] In PluginProcessor.cpp `prepareToPlay()`, call `toneControl.prepareToPlay(sampleRate, samplesPerBlock)`
- [x] In PluginProcessor.cpp `processBlock()`, after the synthesiser renders audio:
  - Read randomized tone values from the active voice (if playing) or base parameter values (if no voice active)
  - Call `toneControl.updateFilters(toneLowGain, toneHighGain)`
  - Call `toneControl.processBlock(buffer)`
- [x] Position the tone processing in the signal chain: after Volume/Pan, before output (since EQ and transient are commented out, Tone effectively replaces them)

### Step 2.4: Add Tone Randomization to Voice

- [x] In RRVoice.h, add member variables:
  - `float randomizedToneLow = 0.0f;`
  - `float randomizedToneHigh = 0.0f;`
- [x] Add getter functions: `getRandomizedToneLow()`, `getRandomizedToneHigh()`
- [x] In RRVoice.cpp `setRandomizationReferences()`, cache pointers for all 6 tone parameters
- [x] In RRVoice.cpp `startNote()`, generate randomized values:
  - `randomizedToneLow` via `RandomizationEngine::generateRandomValue()` using toneLow base + neg/pos ranges
  - Clamp to -12.0 to +12.0 dB
  - Same for `randomizedToneHigh`
- [x] In PluginProcessor.cpp `processBlock()`, read randomized tone values from voice when active (same pattern as the commented-out EQ value reads)

### Step 2.5: Add Tone UI Section

- [x] Add "TONE" section label to the layout
- [x] Position: this will be finalized in Phase 7 (UI Layout), but provisionally place it near the Amplitude section or between Pitch and Sample Start/End
- [x] Add "Low" and "High" knobs using existing RRKnobLAF style
- [x] Add randomization arc overlays for both knobs (blue neg / red pos arcs)
- [x] Add hidden randomization sliders for neg/pos and their APVTS attachments
- [x] Display values as dB with one decimal: "-3.0 dB", "+1.5 dB"
- [x] Wire knobs to APVTS via SliderParameterAttachment

### Step 2.6: State Persistence

- [x] Tone parameters are in APVTS, so DAW project save/load is automatic
- [x] Verify .rrpreset save/load includes tone values
- [x] Old presets (pre-tone) will load with toneLow=0 and toneHigh=0 (no tonal change), which is correct default behavior

**Test:**
- [x] Default state: toneLow=0, toneHigh=0 — audio passes unchanged
- [x] Set toneLow=+6 dB — verify audible bass boost
- [x] Set toneLow=-6 dB — verify audible bass cut
- [x] Set toneHigh=+6 dB — verify audible brightness increase
- [x] Set toneHigh=-6 dB — verify audible brightness decrease
- [x] Both at +12 dB — verify no clipping artifacts or filter instability
- [x] Both at -12 dB — verify signal is very quiet but not silent
- [x] Set toneLow randomization: neg=0.5, pos=0.3 — verify per-note tonal variation
- [x] Rapid triggering — verify no filter clicks or coefficient glitches
- [x] Save/reload project — verify tone values persist
- [x] A/B test against the commented-out ThreeBandEQ low shelf at 250 Hz — verify similar character

---

## Phase 3: Sample Start/End Parameters

**Goal:** Add Start and End percentage-based parameters that trim sample playback boundaries. Include per-note randomization and a fixed micro-fade to prevent clicks when trimming.

### Step 3.1: Register Sample Start/End Parameters

- [x] Add parameter IDs to ParametersIDs.h:
  - `sampleStart` (float, 0.0 to 100.0, default 0.0, displayed as "%" )
  - `sampleEnd` (float, 0.0 to 100.0, default 100.0, displayed as "%")
  - `sampleStartRndNeg`, `sampleStartRndPos` (float, 0.0 to 1.0, default 0.0)
  - `sampleEndRndNeg`, `sampleEndRndPos` (float, 0.0 to 1.0, default 0.0)
- [x] Register all 6 parameters in `createParameterLayout()`
- [x] Update `totalParameters` count in ParametersIDs.h

### Step 3.2: Implement Start/End in Voice Playback

- [x] In RRVoice.h, add member variables:
  - `float randomizedSampleStart = 0.0f;` (percentage)
  - `float randomizedSampleEnd = 100.0f;` (percentage)
  - `int playbackStartSample = 0;` (computed sample index)
  - `int playbackEndSample = 0;` (computed sample index)
- [x] In RRVoice.cpp `setRandomizationReferences()`, cache pointers for all 6 sample start/end parameters
- [x] In RRVoice.cpp `startNote()`:
  - Generate randomized sampleStart and sampleEnd values using RandomizationEngine
  - Scale randomization range: neg/pos values of 0-1 map to 0-50 percentage points offset
  - Clamp: randomized start must be >= 0%, randomized end must be <= 100%
  - Enforce: randomized start must be < randomized end (if they cross, snap start to end - 1%)
  - Convert percentages to sample indices: `playbackStartSample = (int)(randomizedSampleStart / 100.0f * cachedSampleLength)`
  - Same for playbackEndSample
  - Set `sourceSamplePosition = playbackStartSample` instead of 0.0

### Step 3.3: Apply Start/End Boundaries in renderNextBlock

- [x] In RRVoice.cpp `renderNextBlock()`, replace the end-of-sample check:
  - Currently checks: `if (sourceSamplePosition >= cachedSampleLength)`
  - Change to: `if (sourceSamplePosition >= playbackEndSample)`
- [x] Ensure playback begins from `playbackStartSample` (set in startNote)

### Step 3.4: Micro-Fade at Trim Points

- [x] Only apply fade when the user has adjusted start above 0% or end below 100%
- [x] Fade length: fixed at approximately 2-3 ms worth of samples (calculate from sample rate: `fadeSamples = (int)(0.003 * sampleRate)`)
- [x] Fade curve: exponential/logarithmic with a sharp negative shape (approximately -90% curvature) — this means the fade rises very quickly then levels off, avoiding audible clicks while keeping the transient largely intact
- [x] Implementation approach: in `renderNextBlock()`, after reading each sample value, multiply by a fade gain:
  - Fade-in: for samples from `playbackStartSample` to `playbackStartSample + fadeSamples`, apply gain ramp from 0.0 to 1.0 using the curve `gain = pow(linearPosition, 0.1)` (sharp rise)
  - Fade-out: for samples from `playbackEndSample - fadeSamples` to `playbackEndSample`, apply gain ramp from 1.0 to 0.0 using the curve `gain = pow(1.0 - linearPosition, 0.1)` (sharp drop)
- [x] If the trimmed region is shorter than 2x fadeSamples, reduce fade length to half the trimmed region to avoid overlap

### Step 3.5: Add Start/End UI Section

- [x] Add "SAMPLE START/END" section label to the layout (bottom-right area, per mockup)
- [x] Add Start knob and End knob using existing RRKnobLAF style
- [x] Add Start and End randomization arc overlays (RndArcOverlay) — same blue/red arc system
- [x] Add hidden randomization sliders for neg/pos and their APVTS attachments
- [x] Display values as percentages with one decimal: "0.0 %" and "100.0 %"
- [x] Wire knobs to APVTS via SliderParameterAttachment

### Step 3.6: State Persistence

- [x] Ensure sampleStart, sampleEnd, and their randomization values are included in `getStateInformation()` / `setStateInformation()` — this should happen automatically if they are registered in APVTS
- [x] Verify .rrpreset save/load includes the new parameters (existing preset format stores all APVTS values)

**Test:**
- [x] Default state: Start=0%, End=100% — sample plays fully, no fade applied
- [x] Set Start=25% — playback begins at quarter point, no click
- [x] Set End=50% — playback stops at halfway, no click
- [x] Set Start=25%, End=75% — middle 50% plays with micro-fades at both boundaries
- [x] Randomize Start with pos=0.3 — verify per-note variation in start point
- [x] Randomize End with neg=0.2 — verify per-note variation in end point
- [x] Extreme: Start=49%, End=51% — very short playback, no crash, fades overlap gracefully
- [x] Save/reload project — verify start/end values persist
- [x] Save/load .rrpreset — verify start/end values transfer

---

## Phase 4: Sample Manager Panel

**Goal:** Build a comprehensive sample management UI in the left half of the plugin. Includes a scrollable sample list with per-sample actions (reorder, audition, replace, delete), an "add more" clickable area, and the relocated Load Samples button and playback toggle.

### Step 4.1: Create SampleManagerPanel Component

- [x] Create new files: `Source/UI/SampleManagerPanel.h` and `Source/UI/SampleManagerPanel.cpp`
- [x] Add both files to the `target_sources()` list in `CMakeLists.txt` and re-run CMake to regenerate the VS project
- [x] SampleManagerPanel inherits from `juce::Component`
- [x] Takes a reference to the processor (for accessing sample slots, sample loader, and synthesiser)
- [x] Occupies roughly the left 45-50% of the plugin canvas (below the header)

### Step 4.2: Panel Header Area

- [x] "Sample Pool" section title label at top of panel
- [x] "Load Samples" button — relocated from the old header area. Opens multi-file picker (same behavior as current implementation)
- [x] "playback type" label with Series/Random toggle — relocated from old header. Same APVTS attachment to `playbackMode`
- [x] Layout: Load Samples button on the left, playback toggle on the right of the panel header

### Step 4.3: Sample List Display

- [x] Create a scrollable list area below the panel header
- [x] Display loaded samples in a two-column layout (01-10 on left, 11-20 on right) to match the mockup
- [x] Each row shows: `XX. samplename.ext` where XX is the slot number (01-20)
- [x] Sample name truncation: if the filename exceeds maximum display width, truncate to show the first ~6 characters + "..." + the file extension. Example: `sampl0...wav`
- [x] Maximum character length for display: determine based on column width, approximately 14-16 characters total including the number prefix
- [x] Empty slots are not displayed — the list only shows loaded samples
- [x] If fewer than 20 samples loaded, show `--- click to add XX samples ---` text below the last sample in the list, where XX is the number of empty slots remaining
- [x] Clicking the "add more" text opens the file picker (same as Load Samples but appends to existing pool)

### Step 4.4: Per-Sample Action Buttons

Each loaded sample row has 4 small icon buttons to the right of the sample name:

- [x] **Arrow Cross (Reorder):** Initiates drag-and-drop reordering. User clicks and holds, then drags to a new position in the list
  - Swap mode: dropping directly on another sample swaps the two samples
  - Insert mode: dropping between two samples inserts the dragged sample at that position, shifting all samples below down by one index
  - After reorder, update the round-robin index and reshuffle the Fisher-Yates array if in Random mode
  - Visual feedback: highlight drop target (swap) or show insertion line between rows (insert)

- [x] **Play Button (Audition):** Clicking plays that specific sample through the full DSP chain (Volume, Pan, Pitch, Tone, Sample Start/End — same processing as a normal trigger)
  - Implementation: temporarily override the round-robin selection to force-play the clicked sample's slot index, then send a synthetic note-on to the synthesiser
  - After playback, the round-robin index should NOT advance (audition is non-destructive to the sequence)
  - The audition should respect the current parameter settings (volume, pitch, tone, start/end, randomization)

- [x] **Replace Button (Swap File):** Opens a single-file picker dialog. The selected file replaces the audio data in that slot
  - The slot index and position in the pool remain the same
  - The display name updates to reflect the new file
  - The round-robin index is unaffected

- [x] **Trash Can (Delete):** Removes the sample from the pool
  - All samples below the deleted slot shift up to fill the gap (auto-reshuffle)
  - Round-robin index resets to 0 to avoid out-of-bounds
  - Fisher-Yates shuffle array rebuilds
  - The "add more" counter updates accordingly
  - No confirmation dialog (simple one-click delete — the file is not deleted from disk, just removed from the pool)

### Step 4.5: Implement Drag-and-Drop Reorder Logic

- [x] In the processor (or a new helper class), implement `swapSamples(int indexA, int indexB)` — swaps audio buffers, display names, and file paths between two SampleSlot entries, then calls `updateSynthesiserSounds()`
- [x] Implement `insertSample(int fromIndex, int toIndex)` — removes sample from `fromIndex`, inserts at `toIndex`, shifts all intermediate samples accordingly, then calls `updateSynthesiserSounds()`
- [x] Both operations must be thread-safe: the audio thread must not be reading sample data while the UI thread modifies the slots. Use the same locking pattern already in place for sample loading
- [x] After any reorder, reset the round-robin counter and rebuild the shuffle array

### Step 4.6: Wire File Operations

- [x] "Load Samples" button: opens multi-file picker, loads files sequentially into empty slots starting from the first available index
- [x] "Add more" text click: same behavior as Load Samples
- [x] Replace button: opens single-file picker, replaces audio data in that specific slot via `SampleLoader::loadSample(slotIndex, file)`
- [x] Delete button: calls `SampleLoader::clearSlot(slotIndex)`, then shifts remaining samples up and calls `updateSynthesiserSounds()`
- [x] All file operations trigger a repaint of the SampleManagerPanel to reflect the updated list

### Step 4.7: State Persistence for Sample Order

- [x] Sample order is already persisted through `getStateInformation()` which saves file paths per slot index — the order of slots IS the order
- [x] After drag-and-drop reorder, the new slot arrangement persists automatically on next save
- [x] Verify: preset save/load preserves the reordered sample arrangement

**Test:**
- [x] Load 5 samples — verify they appear as 01-05 in list with correct names
- [x] Verify "--- click to add 15 samples ---" appears below sample 05
- [x] Click "add more" — verify file picker opens, new samples append after 05
- [x] Long filename (e.g., "my_very_long_sample_name_footstep_wood.wav") — verify truncation
- [x] Click Play on sample 03 — verify it plays through DSP chain
- [x] Click Replace on sample 02 — verify file picker opens, sample updates
- [x] Click Trash on sample 03 — verify deletion, samples 04/05 shift to 03/04
- [x] Drag sample 01 onto sample 04 (swap) — verify they trade places
- [x] Drag sample 05 between 01 and 02 (insert) — verify proper reindexing
- [x] After reorder, play via MIDI — verify round-robin uses new order
- [x] Save/reload project — verify sample order persists

### Step 4.8: Post-test Edits and Revision

- [x] Enable feature so when user's pointer hovers over truncated name for a couple seconds (or whatever is the standard amount of time is) a popup window appears with the full file name
- [x] Make the sample pool window's audio files be arranged in a single row of 20 samples instead of two rows of 10 samples to makre more room for the sample's text. Include the same four features to the right of the text and continue to truncate if text length exceeds space alloted space. 
- [x] When deleting a sample for the pool the "click here to add more sample" text button is moving up with each deleted sample and is under the text (and moved to the left column even when there are still samples in the right column). This may resolve itself after task two is completed but check logic anyways to ensure a smooth conistent movement with each sample.

---

## Phase 5: Trigger Button

**Goal:** Add a "Trigger" button to the header that fires a synthetic MIDI note-on, advancing the round-robin sequence exactly as if the user played a note on a MIDI keyboard.

### Step 5.1: Add Trigger Button to Header

- [x] Add a `juce::TextButton` labeled "Trigger" in the top-right area of the header bar (to the left of Load Preset / Save Preset / ?)
- [x] Style it with a distinct color (red background per mockup) so it stands out as a play action
- [x] Position: right-aligned in header, before the preset buttons

### Step 5.2: Implement Trigger Logic

- [x] On click, the Trigger button must inject a synthetic MIDI note-on message into the processor's MIDI pipeline
- [x] Use one of the existing trigger notes from MidiMapper (e.g., C1 / MIDI 36) with a fixed velocity (e.g., 0.8)
- [x] The note-on must go through the same `processBlock()` path as real MIDI input so that:
  - The round-robin index advances normally
  - Randomization generates fresh per-note values
  - All DSP processing applies (Volume, Pan, Pitch, Tone, Sample Start/End)
- [x] Implementation approach: store a flag or a small MIDI buffer in the processor. When the editor's Trigger button is clicked, set the flag. In the next `processBlock()` call, if the flag is set, inject the note-on into the MIDI buffer before processing, then clear the flag
- [x] Follow immediately with a note-off after a short delay (or rely on one-shot behavior where note-off is ignored)

### Step 5.3: Trigger and Round-Robin Continuity

- [x] The Trigger button shares the same round-robin sequence as MIDI keyboard input
- [x] If the user plays sample 01 via MIDI keyboard (Series mode), then clicks Trigger, sample 02 plays. If they then hit the MIDI keyboard again, sample 03 plays
- [x] This is automatic as long as both paths go through the same synthesiser noteOn mechanism

**Test:**
- [x] Click Trigger with samples loaded — verify audio plays
- [x] Click Trigger 5 times in Series mode — verify samples cycle 01, 02, 03, 04, 05
- [x] Play MIDI note, then Trigger, then MIDI note — verify continuous sequence (no repeats, no skips)
- [x] Click Trigger with no samples loaded — verify no crash, silent
- [x] Click Trigger in Random mode — verify random selection (no immediate repeat per Fisher-Yates rules)
- [x] Trigger respects current Volume/Pan/Pitch/Tone/Start-End settings and randomization

---

## Phase 5.1: Alter Random Paramater Arc UI

**Goal:** The current UI of the neagtive/positive paramater arcs with the white dots are becoming a UX issue. The fluuidt of changing these paramaters in a circular motion is getting frusterating so would like to redesign the UI and keep all logic the same

### Step 5.1.1: Create and Place New Randomize UI Slider 
- [x] Following a similar aestetic to the current desgign, create a new horizontel slider where in the center two parameter markers will be joined. When user moves the left one to the left, the negative randomization of the parameter's value will increase and when user moves the right paramater marker to the right, the positive randomization will increase. Image refernces for slider configuration are located at /ImageReferences/RanomSlider 0%.png and /ImageReferences/RanomSlider 100%.png. 
- [x] Color of dual slider markers will be consistent with that parameter's color scheme 
- [x] New random slider knobs will be located between the bottom of the parameter knob and the top of the parameter value window. Image reference for slider position is located at /ImageReferences/RandomSliderPostion.png
- [x] Get rid of the current random parameter arc UI around applicable paramater knobs

### Step 5.1.2: Create Corresponding Random Parameter Amount at Location of Former Parameter Arc (IF this is possible using JUCE)
- [x] When user moves new parameter negative slider to the left, there will be colored outline that starts to appear on the outer parameter the paramter knob (where the arc used to be) that will represent the percentage of randomization for that particualr parameter that moves counter clockwise. Same will be the case when user moves positive randomize slider to the right but clockwise. Color of these parameter amounts will align with parameter's color scheme. Image reference is located at /ImageReferences/RandomSliderArcOutlinePosition.png

---

## Phase 6: Random Algorithm Knob

**Goal:** Add a large "Random Algorithm" knob in the center of the plugin that provides a quick way to add progressive randomization across all active parameters. The knob has 18 discrete tick positions, from zero additional randomization to moderate-heavy randomization.

### Step 6.1: Define the Random Algorithm Parameter

- [x] Add parameter ID to ParametersIDs.h: `randomAlgorithm` (int, 0 to 17, default 0)
- [x] Register in `createParameterLayout()` as an integer parameter with 18 steps
- [x] Position 0 = no additional randomization (knob adds nothing)
- [x] Positions 1-17 = progressively increasing randomization amounts

### Step 6.2: Design the Randomization Scaling Table

Each tick position defines an additive offset applied to each parameter's randomization range. These offsets are ADDED to whatever the user has manually set on the individual neg/pos randomization sliders. The values below represent the additional randomization offset at each parameter for each tick:

**Scaling philosophy:** No scaling philosophy now. Please follow custom settings below 

- [x] Tick-to-randomParameter mapping:
- Tick 0: (off)
- Tick 1: volumeRndNeg = 0.0600 / volumeRndPos = 0.0600 / fineTuneRndNeg = 8.0000 / fineTuneRndPos = 8.0000
- Tick 2: volumeRndNeg = 0.1500 / volumeRndPos = 0.1500 / fineTuneRndNeg = 14.0000 / fineTuneRndPos = 14.0000
- Tick 3: volumeRndNeg = 0.1500 / volumeRndPos = 0.1500 / fineTuneRndNeg = 14.0000 / fineTuneRndPos = 14.0000 / toneHighRndNeg = 0.1000 / toneHighRndPos = 0.1000
- Tick 4: volumeRndNeg = 0.1500 / volumeRndPos = 0.1500 / fineTuneRndNeg = 14.0000 / fineTuneRndPos = 14.0000 / toneHighRndNeg = 0.1000 / toneHighRndPos = 0.1000 / toneLowRndNeg = 0.1000 / toneLowRndPos = 0.1000
- Tick 5: volumeRndNeg = 0.2000 / volumeRndPos = 0.2000 / fineTuneRndNeg = 20.0000 / fineTuneRndPos = 20.0000 / toneHighRndNeg = 0.1000 / toneHighRndPos = 0.1000 / toneLowRndNeg = 0.1000 / toneLowRndPos = 0.1000
- Tick 6: volumeRndNeg = 0.2000 / volumeRndPos = 0.2000 / fineTuneRndNeg = 20.0000 / fineTuneRndPos = 20.0000 / toneHighRndNeg = 0.2000 / toneHighRndPos = 0.2000 / toneLowRndNeg = 0.2000 / toneLowRndPos = 0.2000
- Tick 7: volumeRndNeg = 0.2000 / volumeRndPos = 0.2000 / fineTuneRndNeg = 40.0000 / fineTuneRndPos = 40.0000 / toneHighRndNeg = 0.2500 / toneHighRndPos = 0.2500 / toneLowRndNeg = 0.2500 / toneLowRndPos = 0.2500
- Tick 8: volumeRndNeg = 0.2500 / volumeRndPos = 0.2500 / fineTuneRndNeg = 45.0000 / fineTuneRndPos = 45.0000 / toneHighRndNeg = 0.3000 / toneHighRndPos = 0.3000 / toneLowRndNeg = 0.3000 / toneLowRndPos = 0.3000
- Tick 9: volumeRndNeg = 0.3000 / volumeRndPos = 0.3000 / fineTuneRndNeg = 60.0000 / fineTuneRndPos = 60.0000 / toneHighRndNeg = 0.3000 / toneHighRndPos = 0.3000 / toneLowRndNeg = 0.3000 / toneLowRndPos = 0.3000
- Tick 10: volumeRndNeg = 0.3000 / volumeRndPos = 0.3000 / fineTuneRndNeg = 100.0000 / fineTuneRndPos = 100.0000 / toneHighRndNeg = 0.3500 / toneHighRndPos = 0.3500 / toneLowRndNeg = 0.3500 / toneLowRndPos = 0.3500
- Tick 11: volumeRndNeg = 0.3500 / volumeRndPos = 0.3500 / fineTuneRndNeg = 100.0000 / fineTuneRndPos = 100.0000 / toneHighRndNeg = 0.3800 / toneHighRndPos = 0.3800 / toneLowRndNeg = 0.3800 / toneLowRndPos = 0.3800 / sampleStartRndPos = 0.2000
- Tick 12: volumeRndNeg = 0.3500 / volumeRndPos = 0.3500 / semitoneRndNeg = 2.0000 / semitoneRndPos = 2.0000 / fineTuneRndNeg = 10.0000 / fineTuneRndPos = 10.0000 / toneHighRndNeg = 0.3900 / toneHighRndPos = 0.3900 / toneLowRndNeg = 0.3900 / toneLowRndPos = 0.3900 
- Tick 13: volumeRndNeg = 0.3500 / volumeRndPos = 0.3500 / semitoneRndNeg = 3.0000 / semitoneRndPos = 3.0000 / fineTuneRndNeg = 10.0000 / fineTuneRndPos = 10.0000 / toneHighRndNeg = 0.4500 / toneHighRndPos = 0.4500 / toneLowRndNeg = 0.4500 / toneLowRndPos = 0.4500 
- Tick 14: volumeRndNeg = 0.4000 / volumeRndPos = 0.4000 / semitoneRndNeg = 4.0000 / semitoneRndPos = 4.0000 / fineTuneRndNeg = 30.0000 / fineTuneRndPos = 30.0000 / toneHighRndNeg = 0.4500 / toneHighRndPos = 0.4500 / toneLowRndNeg = 0.5000 / toneLowRndPos = 0.5000 
- Tick 15: volumeRndNeg = 0.4000 / volumeRndPos = 0.4000 / semitoneRndNeg = 4.0000 / semitoneRndPos = 4.0000 / fineTuneRndNeg = 40.0000 / fineTuneRndPos = 40.0000 /toneHighRndNeg = 0.4000 / toneHighRndPos = 0.4000 / toneLowRndNeg = 0.5000 / toneLowRndPos = 0.5000 / sampleStartRndPos = 0.1200
- Tick 16: volumeRndNeg = 0.4000 / volumeRndPos = 0.4000 / semitoneRndNeg = 4.0000 / semitoneRndPos = 5.0000 / fineTuneRndNeg = 50.0000 / fineTuneRndPos = 45.0000 / toneHighRndNeg = 0.5000 / toneHighRndPos = 0.5000 / toneLowRndNeg = 0.5000 / toneLowRndPos = 0.5000 / sampleStartRndPos = 0.1200
- Tick 17: volumeRndNeg = 0.4500 / volumeRndPos = 0.4500 / semitoneRndNeg = 4.0000 / semitoneRndPos = 6.0000 / fineTuneRndNeg = 50.0000 / fineTuneRndPos = 50.0000 / toneHighRndNeg = 0.5500 / toneHighRndPos = 0.5500 / toneLowRndNeg = 0.5500 / toneLowRndPos = 0.5500 / sampleStartRndPos = 0.1200 / panRndNeg = 0.1500 / panRndPos = 0.1500



### Step 6.3: Implement Algorithm Offset Application

- [x] In RRVoice.cpp `startNote()`, after generating all randomized values from the user's manual randomization ranges, apply the Random Algorithm offset as an additional layer
- [x] Read the `randomAlgorithm` parameter value (0-17)
- [x] Look up the tick multiplier from the table
- [x] For each affected parameter, generate a second random offset within the algorithm's range and add it to the already-randomized value
- [x] The algorithm offset uses the same RandomizationEngine (uniform random within +/- range)
- [x] The algorithm always applies symmetrically (equal neg and pos range) — it does not respect the user's asymmetric neg/pos settings for its own contribution
- [x] Clamp final values to valid parameter ranges after applying both user randomization and algorithm offset

### Step 6.4: Add Random Algorithm Knob to UI

- [x] Create a larger knob component for the Random Algorithm (approximately 1.5-2x the size of standard parameter knobs per the mockup)
- [x] Position: center of the plugin, to the right of the Sample Manager panel, above the Pitch section
- [x] Label: "Random Algorithm" section title above the knob
- [x] The knob should snap to 18 discrete positions (use `setNumDecimalPlacesToDisplay(0)` and integer slider range)
- [x] Visual: 18 tick marks around the knob to indicate discrete positions
- [x] Use existing RRKnobLAF but scaled up, or create a variant for the larger knob
- [x] No randomization arc on this knob (it IS the randomization control)

### Step 6.5: State Persistence

- [ ] The `randomAlgorithm` parameter is in APVTS, so it persists in DAW projects automatically
- [ ] Verify it saves/loads with .rrpreset files

**Test:**
- [x] Set Random Algorithm to tick 0 — verify no additional randomization beyond user's manual settings
- [x] Set to tick 1 — verify barely perceptible variation (A/B against tick 0 to hear difference)
- [x] Set to tick 9 — verify moderate, musical randomization across all parameters
- [x] Set to tick 17 — verify heavy but not chaotic randomization
- [x] Set user's manual Volume randomization to +3dB pos, then set Algorithm to tick 9 — verify the algorithm adds on top (total randomization exceeds +3dB)
- [x] Verify Tone Low/High randomization is light and musical even at tick 17
- [x] Verify Sample Start randomization is very light even at tick 17 (transients not destroyed)
- [x] Save/reload — verify tick position persists

---

## Phase 7: UI Layout, Polish, and Integration

**Goal:** Final layout pass to integrate all new components into the 700x450 canvas. Wire everything together, ensure visual consistency, and perform integration testing.

### Step 7.1: Final Layout Planning

- [ ] Update Figma mockup (or sketch) to reflect all changes:
  - Header bar: Plugin name (left), Trigger button, Load Preset, Save Preset, ? (right)
  - Left half (~45-50%): Sample Manager panel (Load Samples, playback toggle, sample list with actions, "add more" text)
  - Center: Random Algorithm knob (large)
  - Top-right: Amplitude section (Volume + Pan knobs with randomization arcs)
  - Center-right: Pitch section (Semitone + Fine Tune knobs with randomization arcs)
  - Center-right (below Pitch or beside it): Tone section (Low + High knobs with randomization arcs)
  - Bottom-right: Sample Start/End section (Start + End knobs with randomization arcs)
  - Footer: company name

### Step 7.2: Implement Final resized() Layout

- [ ] Update PluginEditor.cpp `resized()` with new layout constants
- [ ] Position SampleManagerPanel in the left portion of the canvas
- [ ] Position Random Algorithm knob centered between the Sample Manager and the parameter sections
- [ ] Position Amplitude, Pitch, Tone, and Sample Start/End sections stacked vertically on the right
- [ ] Ensure all randomization arc overlays are properly positioned over their respective knobs
- [ ] Verify no component overlaps

### Step 7.3: Update paint() for New Sections

- [ ] Update `paint()` to draw section backgrounds and borders for: Sample Pool panel area, Random Algorithm area, Amplitude, Pitch, Tone, Sample Start/End
- [ ] Draw section title labels: "Sample Pool", "Random Algorithm", "AMPLITUDE", "PITCH", "TONE", "SAMPLE START/END"
- [ ] Draw the yellow divider line between sections (per existing style)
- [ ] Update knob labels: "Volume", "Pan", "Semitone", "Fine Tune", "Low", "High", "Start", "End"
- [ ] Update footer

### Step 7.4: Verify State Persistence End-to-End

- [ ] Load samples, set all parameters (Volume, Pan, Pitch, Tone, Start/End with randomization, Random Algorithm position, playback mode)
- [ ] Save DAW project, close, reopen — verify full state restoration
- [ ] Save .rrpreset — verify all new parameters included
- [ ] Load .rrpreset on a fresh instance — verify complete state (including sample order)
- [ ] Load an OLD .rrpreset (pre-redesign) — verify graceful handling: new parameters get defaults, old EQ/transient/envelope values ignored without error

### Step 7.5: Edge Cases and Robustness

- [ ] Empty pool: Trigger button, Play button, MIDI input — all silent, no crash
- [ ] Single sample loaded: Series cycles through just that one sample repeatedly
- [ ] All 20 samples loaded: list displays correctly in two columns, "add more" text hidden
- [ ] Rapid trigger clicking: no audio glitches, voice cuts off previous and restarts
- [ ] Sample Start near Sample End with randomization: verify clamp logic prevents crossover
- [ ] Random Algorithm at max + user randomization at max: verify values stay within valid parameter ranges
- [ ] Delete all samples one by one via trash button: verify clean state, no dangling references
- [ ] Drag reorder with only 2 samples loaded: verify swap works

### Step 7.6: Performance Validation

- [ ] CPU usage remains reasonable (Tone is lightweight — two shelf filters)
- [ ] No audio dropouts at 128-sample buffer size
- [ ] UI remains responsive during sample loading and drag operations
- [ ] Memory usage stable after repeated load/delete cycles

**Test (Integration):**
- [ ] Full workflow: Load 10 samples -> set Series -> adjust Volume/Pan/Pitch/Tone -> set Start to 10%, End to 90% -> set Random Algorithm to tick 5 -> click Trigger 10 times -> verify each trigger sounds slightly different with consistent quality
- [ ] Reorder samples via drag -> verify Trigger plays in new order
- [ ] Delete sample 03 -> verify list updates, Trigger continues working
- [ ] Replace sample 05 -> verify new audio plays at that position
- [ ] Audition sample 07 via Play button -> verify it plays but does not advance round-robin
- [ ] Switch to Random mode -> Trigger 20 times -> verify no immediate repeats (Fisher-Yates)
- [ ] Save preset -> close plugin -> load preset -> verify identical state

---

## Phase 8: Ongoing Edits and Revisions

**Goal:** Ongoing issues, bugs and revisions to work on

### Step 8.1: Code Revisions and Additions
- [x] We need to change the sample start logic so the settings are relative to the longest sample in the sample pool. For short samples, for example, when sampleStart = 48.5000, the sample becomes in audible as the start point is beyond the actualy length of the sample. Can you and is it possible to adjust the samples start amount so that it is the length of the longest samples in the pool? 
- [x] I'd like to adjust Step 5.1.2: Create Corresponding Random Parameter Amount at Location of Former Parameter Arc. When the user moves the random parameter slider, I'd like for the parameter arc outline to adjust relative to the paramter knob's location which, I believe, reflects how the randomize logic is effected when its parameter position is changed. Take special care with the volume knob as its default location is set to 2.5db.  

### Step 8.2: Bugs

### Step 8.3: UI 
- [x] Redesign the positioning of the UI based on the uploaded image 'VST_RRLite_v3'. This is a mockup so do not adjust sections, parameters, windows, etc exactly to the image but rather move everything around so it roughly aligns with it but use design theory to fine tune where the sections are located. For this you will have to change the dimensions of the plugin so the width exceeds the height. 
- [x] I would like for you to change the general aestetic so it feels like MainInspo as your northstar and GeneralInspo_01 and GeneralInspo_02 support. NOT exactly like these images but has a pseudo-vintage feel with a modern shein. This step includes finding a good color pallete that's similar to what we have going but try and improve the style of the pallete. 
- [x] For the sample browser, I want you to do your best to emulate the LED screen of the Eventide Orville. Blue backround with white text font. You do not have to do copy the exact colors ig they do not match well with the color pallete at this stage, but try and keep the colors close to the refernce. Image reference is called 'LEDScreen' 

### Step 8.4: Sample Length and Start/Stop Issue - Change envelope behavior/sample start and end logic
- [ ] Envelope removed — envelopeLevel no longer multiplied into output; voice terminates immediately when it reaches playbackEndSample. Samples now play start-to-stop at full amplitude regardless of length. The juce::ADSR member and envelope.noteOn() call are still present but dormant (no behavior), so Phase 5's  transient shaper can reuse or replace the envelope cleanly later.   
- [ ] Start/End relative to longest pool sample — both percentages now reference maxPoolSampleLength and clamp to each individual sample's length. For a 40-second  sample alongside a 1-second sample: End=50% cuts both at the 20-second mark (so the 1-second one plays fully, the 40-second one cuts at 20s).         

### Step 8.5: Add mp3 Compatability 
- [ ] Enable plugin to import mp3 files in addition to .wav, .aif, etc 

---

## Parameter Summary (Lite Redesign)

| Category | Parameter | Range | Default | Randomized | In UI |
|---|---|---|---|---|---|
| Pitch | semitone | -12 to +12 | 0 | Yes | Yes |
| Pitch | fineTune | -100 to +100 cents | 0 | Yes | Yes |
| Amplitude | volume | 0.0 to 1.0 (gain) | ~0.75 (-2.5dB) | Yes | Yes |
| Amplitude | pan | -1.0 to +1.0 | 0 | No | Yes |
| Tone | toneLow | -12.0 to +12.0 dB | 0 | Yes | Yes |
| Tone | toneHigh | -12.0 to +12.0 dB | 0 | Yes | Yes |
| Sample | sampleStart | 0 to 100 (%) | 0 | Yes | Yes |
| Sample | sampleEnd | 0 to 100 (%) | 100 | Yes | Yes |
| Algorithm | randomAlgorithm | 0-17 (int) | 0 | No | Yes |
| Playback | playbackMode | 0/1 (Series/Random) | 0 | No | Yes |

**Commented-out parameters (kept in code for Premium):**
- EQ: lowGain, lowFreq, midGain, midFreq, highGain, highFreq (+ 12 rnd params)
- Transient: transientAttack, transientDecay (+ 4 rnd params)
- Envelope: envAttack, envDecay (+ 4 rnd params)

**Active APVTS parameters:** 10 base + 12 randomization (neg/pos pairs for semitone, fineTune, volume, toneLow, toneHigh, sampleStart, sampleEnd — note: pan rnd deferred) = ~22 parameters

---

## Estimated Development Effort

| Phase | Description | Estimate |
|---|---|---|
| 1 | UI Simplification & Parameter Cleanup | 1-2 sessions |
| 2 | Tone Control (Low/High Shelf) | 1-2 sessions |
| 3 | Sample Start/End Parameters | 2-3 sessions |
| 4 | Sample Manager Panel | 3-5 sessions |
| 5 | Trigger Button | 1 session |
| 6 | Random Algorithm Knob | 2-3 sessions |
| 7 | UI Layout, Polish & Integration | 2-3 sessions |
| **Total** | | **~13-19 sessions** |

---

## File Changes Summary

**New files:**
- Source/DSP/ToneControl.h
- Source/DSP/ToneControl.cpp
- Source/UI/SampleManagerPanel.h
- Source/UI/SampleManagerPanel.cpp

**Heavily modified:**
- Source/PluginEditor.h (remove commented components, add new ones)
- Source/PluginEditor.cpp (new layout, new sections, Sample Manager integration)
- Source/PluginProcessor.cpp (comment out DSP calls, add ToneControl, add trigger flag, add new parameters)
- Source/PluginProcessor.h (add ToneControl member, add trigger flag, Sample Manager accessors)
- Source/Audio/RRVoice.h (add tone/start/end members, comment out removed getters)
- Source/Audio/RRVoice.cpp (add tone/start/end logic, micro-fade, algorithm offset, comment out removed randomization)
- Source/Parameters/ParametersIDs.h (add new IDs, keep old ones)

**Unchanged (code stays, just unused in Lite):**
- Source/DSP/ThreeBandEQ.h/.cpp
- Source/DSP/TransientShaper.h/.cpp
- Source/DSP/RandomizationEngine.h/.cpp
- Source/Audio/RRSound.h/.cpp
- Source/Audio/MidiMapper.h
- Source/Data/SampleSlot.h

**CMake Build Command:**
- cd /Users/alex/Documents/Github/round-robin-lite/NewProject
cmake -B Builds/MacOSX -G Xcode -S .
