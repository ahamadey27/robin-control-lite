# AGENTS.md

This file provides guidance to Codex (Codex.ai/code) when working with code in this repository.

For company-level brand, business, marketing, commerce, privacy, product-portfolio, and cross-repository context, read `/Users/alex/Documents/Github/conduit-dsp-website/CONDUIT_DSP_CONTEXT.md` on this Mac, or `../conduit-dsp-website/CONDUIT_DSP_CONTEXT.md` when that sibling is available on another machine. Its absence on a fresh Windows clone does not block building this repository. This repository's `AGENTS.md`, current code, release checklists, and test evidence remain authoritative for Robin Control Lite implementation details.

## Project

**Windows 2.0.1 continuation — October 25:** GitHub main was merged with the
existing Windows release history, preserving installer/signing tools and the
editor-owned software logo correction. Read `WINDOWS_2.0.1_BUILD.md` for native
build, validation, signing, packaging and remaining manual checkpoints. The
merged `scripts/test-aax-native.py` supports Windows with explicit `--dsh`;
use `WINDOWS_BETA_2.0.0.md` for the preference-preserving toolkit configuration.
Later Mac-only script descriptions below reflect that checkout's history.

**macOS 2.0.1 test installer — October 23:** Alex authorized a full Mac test
package. Current source version is **2.0.1**, unpublished; public Lite remains
2.0.0. Read `MACOS_2.0.1_TEST_CANDIDATE.md` for the signed/notarized/stapled
universal VST3/AU/AAX installer and checksum. Fresh final-profile builds,
engine/licensing tests, ASan/UBSan/TSan, both-slice exports, native/Intel signed
VST3/AU GUI validation and 13/13 unsigned/signed AAX checks pass. The final
extracted payload matches staging; Gatekeeper accepts it. Existing installed
bundles were preserved during build validation; temporary AU registration
was restored and verified. Alex subsequently installed 2.0.1 and confirms the
PatchWork retest passed. All three installed system payloads independently
match signed 2.0.1 staging. Pro Tools is closed; the actual wrapper-loaded path
during that test was not independently observed. See `local-package-acceptance.json`.
Affected-customer acceptance, remaining host/installer scenarios, Windows 2.0.1
and publication remain pending. Do not repeat the completed local test by default.

**October 23 native AAX continuation:** Alex confirms native mono/stereo MIDI,
Series/Random, editor/playback, insert removal/reinsertion and save/quit/reopen
all completed without errors. Fresh ARM64 Pro Tools PID 72164 maps the exact
signed combined-candidate AAX; the corrected VST3 is also mapped. Actual module
unloading is unmeasured. **The second temporary diagnostic AAX installation is
finished; the original is restored.** Complete file hashes/modes/symlinks and
independent Apple/PACE verification pass; the protected backup directory was
removed and the tested candidate is archived in ignored evidence. Pro Tools is
closed. Read the tracker and `pro-tools-native-midi-unload-20261023/` evidence.
Affected-customer exact-candidate acceptance and separate startup evidence
remain pending; this is local host evidence, not a published hotfix.

**Pro Tools reliability work (2026-10-22):** Alex authorized the phased RCL/RC
plan in `PRO_TOOLS_RELIABILITY_PLAN_2026-10-22.md`; read it before continuing.
The customer dump confirms a static `juce::emptyString` invalid free during
VST3 editor construction inside PatchWork / Pro Tools. The focused visibility
candidate is in `NewProject/build-pt-visibility-20261022`, version still 2.0.0,
Moonbase/final ON and auto-copy OFF. All three universal formats build; every
slice passes the new internal-export/entry-point gate, and native VST3 pluginval
strictness 10 including GUI passes. The continuation installed Apple Rosetta 2;
Intel execution works again. Fresh signed combined-candidate VST3/AU pass native
and Intel validation including GUI, with exact AU identification and verified
restoration of all prior user/system AUs. RCL AAX passes all 13 applicable tests
before/after its own PACE Signing Only; Apple/PACE signatures and both-slice
exports pass. No permanent plug-in replacement, package or hotfix publication.
Evidence is under `Releases/Testing/2.0.0/pro-tools-audit-20261022/` (ignored).
Preserve the F1-only candidate. Pool locking and XML preflight are now implemented
in the separate `NewProject/build-pt-reliability-20261022` combined candidate;
all universal export checks and native strictness-10 VST3 GUI tests pass. Fresh
engine/licensing tests and ASan/UBSan/TSan pass with the new pool-operation and
malformed-state regressions. Source/artifact hashes match committed `65dc245`.
ARM64 retail Pro Tools 26.4.1.179 cold launch and the mono/stereo editor/sample/
Trigger/Panic test passed; Alex also confirms saved-session and sample restore.
The temporary signed diagnostic AAX has been removed and the original system
bundle restored with complete inventory and independent Apple/PACE verification.
PatchWork AAX/Synth 2.78 is installed by Alex. Lite's corrected VST3 now passes
sample/Trigger/Panic, editor cycles and remove/reload inside PatchWork Synth
(user-confirmed). The live mapped path/inode/hash/ARM64 UUID matches the signed
copy at `/private/tmp/RCL-PatchWork-Test-20261022/Robin Control Lite.vst3`;
Pro Tools 26.4.1.179 ARM64 / macOS 27.0.1 are verified. System VST3 and restored
native AAX remain the originals. This local combined-candidate pass does not
establish the affected customer's outcome. Alex now also confirms MIDI playback,
Add/Replace/Delete/Clear/reload during playback, Series/Random and a minute of
editor-closed playback/reopen pass; live candidate path/inode/hash still matches.
The full wrapper save/quit/reopen check now also passes (user-confirmed):
samples/order/settings/MIDI restore, and fresh ARM64 Pro Tools PID 67404 maps
the exact same corrected VST3. New startup trace still records 48 kHz / 256.
The 48 kHz offline bounce now completes without errors; Avid's trace confirms
Offline plus buffer changes 64/512/128 → 256. The stereo 24-bit 32.36 s WAV
decodes with signal and is preserved/hash-verified in ignored evidence. Alex
explicitly confirms the WAV and MIDI playback at both 64/512 sounded normal;
those fields now pass. The initial 96k-named export/active session were still
48 kHz; this setup mismatch is preserved. Alex corrected the session setup, and
regular-host trace now verifies the actual 48 → 96 kHz switch at 256 samples
and Offline bounce. The same corrected VST3 remains mapped; the new stereo/
24-bit/96 kHz/32.36 s WAV decodes with signal and has a verified evidence copy.
The corrected session/render is user-confirmed successful. Read the tracker and
`host-validation-checkpoint-summary.json` for precise scope and remaining gates.
Historical-session, Windows, remaining host scenarios and affected-customer
acceptance remain pending; read the tracker for the precise scope. RC prevention
has its own build/profile, PACE configuration and separate validation evidence.
Company context records public Mac/Windows Lite 2.0.0 downloads; the older
release-preparation/public-format statements below predate that release.

**Robin Control Lite** is a free monophonic sampler plugin by `conduit.dsp`, built with JUCE 8+ and C++17. Current code version: **2.0.1 (unpublished macOS test candidate)**; public Mac/Windows Lite remains 2.0.0. Read `MACOS_2.0.1_TEST_CANDIDATE.md` for current Mac package evidence and `FINAL_RELEASE_2.0.0.md` for the historical 2.0.0 build. Public 2.0.0 downloads include Mac VST3/AU/AAX and Windows VST3/AAX; no Windows 2.0.1 build is claimed. Automated gates and the earlier diagnostic host passes do not establish exact 2.0.1 installer/customer acceptance. Standalone is for local testing only.

For current final-release work, read **`FINAL_RELEASE_2.0.0.md`** first, then **`RELEASE_2.0.0.md`** (build commands, artifact paths, evidence, and open checkpoints) and **`AAX_BUILD_AND_SIGNING.md`** (AAX/PACE handoff for future agents). These take precedence over historical v1.0 signing instructions in `release-spec.md` for the 2.0.0 release. Do not infer AAX authorization from an installed SDK or WrapTool.

**When cloning onto Windows:** read **`WINDOWS_2.0.1_AGENT_HANDOFF.md`** first, then **`WINDOWS_BUILD_AND_AAX.md`** before configuring. The current handoff records exact production-profile x64 build/test commands, source/dependency pins, Windows F5 artwork review and packaging/signing scope. Clone the private `ahamadey27/robin-control-lite` repo, preserve existing Windows-only work, use a fresh Windows build directory and explicit JUCE/AAX SDK paths, and build x64 with MSVC. Mac binaries, CMake caches, signing credentials, and ignored `Releases/Testing` evidence do not transfer through Git. The current AAX validator/setup scripts are macOS-only; use or deliberately adapt the established native Windows workflow. Do not treat existing Windows VST3 CI as AAX validation.

Windows-specific implementation and signing evidence is preserved in WINDOWS_FINAL_2.0.0.md. The merged native Windows AAX runner supports --dsh; use WINDOWS_BETA_2.0.0.md for toolkit setup. The editor-owned software logo fix is retained.

**Licensing direction, user-confirmed 2026-09-20:** Moonbase DRM is the chosen customer licensing system for this free product and all future products. It is required for the final v2.0.0 release but explicitly excluded from Beta 1 (the user's “beta v1”). PACE is for AAX digital signing only; no customer iLok/PACE DRM. This supersedes the provider choice in older custom-licensing plans, including the July company-context roadmap. Do not block Beta 1 on Moonbase integration or infer a binary version change from the beta label. That provider decision alone did not integrate licensing into any existing candidate. See the dated integration update below for current settings and build status.

**Final-release direction, 2026-09-22:** Moonbase is integrated. Every final build must use `RCL_ENABLE_MOONBASE=ON` **and** `RCL_FINAL_RELEASE=ON`. Development defaults and Projucer remain DRM-free and must not be packaged as final. Alex chose **90 days** of offline grace for online licenses; permanent offline activation is separate. Dashboard: **10 activations**, offline activation ON, trials/auto-upgrade trials OFF. The private **Moonbase Beta 1** was signed/notarized and its Moonbase delivery email/download flow was user-confirmed; do not confuse it with the earlier DRM-free beta. `MOONBASE_INTEGRATION.md` records details. PACE supplies AAX signing only. Do not upload/publish or change commerce settings merely because final binaries are built.

**Robin Control handoff:** `ROBIN_CONTROL_RELIABILITY_HANDOFF_2026-10-23.md` records the current F1–F6 comparison and which existing RC validations can be reused; a copy is in premium. Explicit visibility/export prevention is already applied, and RC already has transaction locking, guarded parsing and editor-owned software artwork. No new RC runtime patch or test rerun was performed by this review. `ROBIN_CONTROL_AGENT_HANDOFF.md` retains general licensing/build/security context. Use premium's own identities, Moonbase policy and PACE configuration; do not copy Lite's version/limits or claim Lite host passes as RC acceptance.

As of 2026-04-25 this folder is the **authoritative project**. It was originally a UI-redesign sandbox forked from `../round-robin-lite`, but has been promoted to be the main codebase. The sibling folder `../round-robin-lite` is retired — kept on disk as read-only legacy reference, not built or installed.

For the full development plan (formats, signing, distribution, CI, testing, license decisions), see `spec.md` at the repo root. **`spec.md` is the source of truth** for anything not covered here. Sibling docs at the repo root:
- `release-spec.md` — **the active v1.0 ship execution playbook**. Sequential, command-level checklist with [x]/[ ] checkboxes covering build config, Apple signing, AAX validation, Pro Tools test, Windows build, distribution. When `spec.md` and `release-spec.md` disagree on something v1.0-specific, `release-spec.md` wins. Update it as you complete items.
- `spec-paperwork.md` — admin/legal checklist (Apple Developer enrollment, trademark, etc.)
- `EULA.md` — end-user license for the binary; establishes **Conduit DSP LLC** (Kingston, NY; Ulster County jurisdiction) as the legal entity. Version 2.0.0 text updated September 22, 2026 for Moonbase; historical v1 terms remain in Git. Will be published at `https://conduitdsp.com/eula/robin-control-lite/`.
- `LICENSE` — source code license: **All Rights Reserved (proprietary)**. The source is not open-source; a Pro version is planned, so the codebase must remain shareable between Lite and Pro without a third-party fork ever shipping.
- `Privacy.md` — short pointer to the canonical privacy policy at `https://conduitdsp.com/privacy-policy/`. Moonbase activation sends device/license information and stores a signed license locally; optional analytics are disabled. Public website policy reconciliation is a launch checkpoint.
- `README.md` — end-user-facing install/use docs

GitHub remote: `https://github.com/ahamadey27/robin-control-lite.git` (private). Local folder name (`robin-control-redesign`) intentionally not changed — VSCode workspace paths still point at the old folder name.

### Product behavior (free version)

The shipped product is **monophonic** and triggers from **any MIDI key**. There is **no** paired-key MIDI mapping, **no** white-keys-only restriction, and **no** chromatic per-key pitching. Pitch is global only (semitone + fine-tune knobs). The codebase still contains paired-key infrastructure (`MidiMapper::NUM_KEY_PAIRS = 10`, `RRSound::keyPairIndex`) inherited from the parent project — that's internal plumbing, not user-facing behavior. **When the inherited code description disagrees with the shipped product, the product wins.** See `memory/project_free_version_scope.md` for full reasoning.

**Private Moonbase beta — 2026-09-22:** Alex explicitly requested a DRM-enabled beta for Moonbase upload and a fresh install. `MOONBASE_BETA_1.md` records the universal, signed/notarized VST3/AU/AAX installer, validation and removal of installed RCL copies. This named Moonbase Beta 1 is separate from the earlier DRM-free Beta 1 scope; do not disable licensing when reproducing this package. Binary version remains 2.0.0. The Moonbase delivery email/download flow was subsequently user-confirmed. Full licensed retail-host and offline activation coverage remains a manual checkpoint; see the final release record.

## Build

```bash
cd NewProject
cmake -B build-final -DCMAKE_BUILD_TYPE=Release -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build-final --config Release
```

JUCE resolution: defaults to `~/Documents/JUCE` if present, else `FetchContent` downloads checksum-pinned JUCE 8.0.15 (commit `91ad83ae34a81e0833b1a2b0866f54846370ae53`). Override the local path with `cmake -DJUCE_PATH=/path/to/JUCE -B build`.

Development builds copy automatically when `RCL_COPY_PLUGIN_AFTER_BUILD=ON`; release commands turn this OFF:
- VST3 → `~/Library/Audio/Plug-Ins/VST3/Robin Control Lite.vst3`
- AU → `~/Library/Audio/Plug-Ins/Components/Robin Control Lite.component`

AAX is built when the SDK is found at `~/SDKs/aax-sdk-2-9-0` (override with `cmake -DJUCE_AAX_SDK_PATH=...`). Explicitly build `RobinControlLite_AAX` so a missing SDK cannot silently omit the format. The unsigned `.aaxplugin` lives in `build/RobinControlLite_artefacts/<Config>/AAX/`; retail Pro Tools requires the separately PACE-signed copy. Follow `AAX_BUILD_AND_SIGNING.md` for signing, validation, and installation. Preserve `Contents/Resources/RobinControlLitePages.xml` and PACE symlinks, and never rebuild over signed staging. Use PACE `sign`, not `wrap`; customer Moonbase licensing is separate.

Standalone is excluded from public releases but available for local testing with `-DRCL_BUILD_STANDALONE=ON` (defaults to OFF). In VS Code, select **Launch Robin Control Lite (Standalone)** and press **F5**; its tasks configure and build the Debug standalone app before launching it with CodeLLDB. The optional **Attach lldb to host** configuration remains available for debugging the AU/VST3 inside a DAW.

The historical development `build` directory uses Debug. Final commands above use Release; exact universal toolchain commands are in `FINAL_RELEASE_2.0.0.md`. JUCE writes artefacts under `<build-directory>/RobinControlLite_artefacts/<Config>/<Format>/`.

Only the Xcode generator produces a `.xcodeproj`; the final Mac build uses Unix Makefiles.

A Projucer file (`NewProject/RobinControlLite.jucer`) is kept in sync with CMake, but **CMake is the authoritative build**. The old `NewProject/Builds/` tree is stale Projucer output — do not rely on it; safe to delete.

### Testing

A layered automated testing platform exists — **`TESTING.md` is the source of truth**. Quick reference:
- `scripts/test-plugin.sh` — local pluginval (strictness 10) + auval. Run before every release.
- `python3 scripts/test-aax-native.py <bundle> --output <new-directory>` — macOS/Windows AAX validation, before and after PACE signing. Windows requires `--dsh <path-to-dsh.exe>` and the toolkit configuration described in `WINDOWS_BETA_2.0.0.md`. Confirms Lite's Native-only descriptor, checks all installed applicable results, and records the DSP/HDX-only cycle-count test as N/A. This runner is scoped to Lite; adapt it deliberately for another product.
- `tests/` — JUCE `UnitTest` console app for pure-logic units (RandomizationEngine, MidiMapper). Build with `cmake -DRCL_BUILD_TESTS=ON` (off by default; does not affect the plugin build).
- `scripts/test-sanitizers.sh` — ASan/UBSan + TSan over the test target (Clang/macOS only).
- `.github/workflows/validate.yml` — CI on every push: builds + validates on **macOS and Windows**, plus unit-test and sanitizer jobs. **Green as of 2026-06-01.**

Still useful for manual checks: JUCE AudioPluginHost or any DAW (the real-host smoke matrix in `TESTING.md §5`). `tests/` now exercises the real processor, malformed state/audio files and resampling, plus a separate DRM-enabled suite using synthetic signed licenses. `scripts/test-sanitizers.sh` runs both under ASan/UBSan and TSan. Keep these protections when changing decoding or activation. Bounds: 50 million decoded floats, 64 channels, finite 8–192 kHz source rate; 8 MiB state/preset limit; 1 MiB license-cache limit. The whole platform is plugin-agnostic and designed to be lifted into the Pro version (`TESTING.md §7`).

### Audio formats supported

WAV, AIFF, FLAC, OGG, **MP3** (decode-only). MP3 is gated behind `JUCE_USE_MP3AUDIOFORMAT=1` in `CMakeLists.txt` (`target_compile_definitions`). With that flag set, `formatManager.registerBasicFormats()` **already registers `MP3AudioFormat`** — so the explicit `registerFormat(new MP3AudioFormat())` in `PluginProcessor`'s constructor is only a fallback for if the flag is ever turned off, and is **guarded against double-registration** (registering the same format twice trips a JUCE assert in Debug and duplicates the reader). Don't remove that guard. JUCE's MP3 decoder ships royalty-free; safe for distribution. The picker filters and `SampleLoader::supportedFormats` array must list `.mp3` too — all three places stay in sync.

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

When the user says **"undo to original"** (or any close variant — "revert to original", "back to original", etc.), treat it as a command to restore the project to the baseline state captured on 2026-04-22: git commit `5843ed5` plus the AGENTS.md edits made that same day. The `original-baseline` git tag marks this state. **Note:** this baseline predates the 2026-04-25 pivot to main project — restoring it reverts identity strings, the spec.md, and AGENTS.md back to the sandbox era. Confirm with the user before committing or discarding anything else.

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
