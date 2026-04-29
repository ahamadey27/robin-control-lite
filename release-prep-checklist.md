# Release Prep Checklist — Robin Control Lite

Technical work to complete while Apple Developer enrollment is in review. No code signing or notarization tasks here — those execute as soon as the certs arrive. Windows-side work also excluded (separate machine).

Cross-references to `spec.md` are inline where the section spells out detail.

---

## 1. Build hygiene

- [x] Fresh-clone CMake configure works without `JUCE_PATH` set (verified: forced `JUCE_PATH=/nonexistent`, FetchContent path resolved JUCE 8.0.4 cleanly, configure done in 25.8s)
- [x] Mac universal Release build succeeds — VST3 + AU + Standalone produced as `Mach-O universal x86_64 + arm64`:
  ```bash
  cd NewProject
  cmake -B build-release -G Xcode \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
  cmake --build build-release --config Release
  ```
- [ ] **Build compiles with zero warnings** — DEFERRED. Current build emits 186 warnings in `Source/` (56 implicit-int-float, 26 unused-private-field [mostly Pro-reserved per `update-premium-spec.md` §2 — fix with `[[maybe_unused]]`, NOT deletion], 26 unused-parameter, 24 sign-conversion, 16 float-conversion, 14 sign-compare, 10 shadow-field-in-constructor, 4 unused-variable, 4 misc). None affect correctness. Address in a focused cleanup pass before tagging `v1.0.0`. See §11 below.
- [x] Version string set consistently — `project(RobinControlLite VERSION 1.0.0)` and `juce_add_plugin(... VERSION 1.0.0 ...)` both in `NewProject/CMakeLists.txt`
- [x] About dialog version stamp — bottom-right corner of `AboutWindow` renders `juce::String("v") + JucePlugin_VersionString`, dim text, doesn't perturb measured layout
- [x] `CMAKE_OSX_DEPLOYMENT_TARGET` default dropped from `13.0` to `11.0` (per spec.md §3.7) so the floor is correct without needing the flag on the command line every time
- [x] `.gitignore` updated — replaced blanket `[Rr]eleases/` (Visual Studio template) with the specific spec.md §9 patterns; added `NewProject/Builds/`
- [x] Stale paths removed — `NewProject/Builds/MacOSX/` (32K) + `NewProject/Builds/VisualStudio2026/` (888K). `design-spec.md` / `RRLite_spec_v2_OLD.md` already gone in a prior session.
- [x] `Releases/macOS/`, `Releases/Windows/`, `Releases/Installers/` folder structure tracked via `.gitkeep`

---

## 2. Automated validation (Mac)

Run on the unsigned Release build — signing doesn't change validation behavior.

- [ ] `pluginval --strictness-level 10 --validate-in-process --timeout-ms 600000` passes on VST3
- [ ] `pluginval --strictness-level 10 --validate-in-process --timeout-ms 600000` passes on AU
- [ ] `auval -v aumu rcll Cdsp` passes with zero warnings
- [ ] Every failure resolved — these are state-recall and threading bug magnets, do not paper over

---

## 3. Tier-1 host smoke (Mac, VST3 + AU)

Run §7.4 + §7.5 + §7.6 in `spec.md` for each host. ~5–7 min/host once warmed up.

- [ ] Logic Pro (AU)
- [ ] Ableton Live 12 (VST3 + AU)
- [ ] Reaper 7 (VST3 + AU)
- [ ] JUCE AudioPluginHost (VST3 + AU — neutral reference, isolates host bugs from plugin bugs)

Highest-value items inside those sections:

- [ ] Plugin name shows as "Robin Control Lite"; manufacturer reads "conduit.dsp"
- [ ] Save host project → close host → reopen → every loaded sample, every knob, every neg/pos randomization range restored exactly
- [ ] VST3 ↔ AU parity: load identical samples + params in both formats, render same MIDI clip → bit-identical (or sub-LSB) output
- [ ] Plugin's own preset save → reload — exact state restored
- [ ] Offline bounce vs realtime — match (catches block-size-dependent state bugs)

---

## 4. Automation tests (Logic, Live, Reaper)

Per §7.5:

- [ ] Volume automation: write → playback recreates moves exactly
- [ ] Randomization neg/pos thumbs automate **independently** (they're hidden APVTS sliders driven by `DualThumbRndSlider` — both sides need their own automation lane)
- [ ] Sample Start / Sample End percentage automation
- [ ] Save → reopen — automation lanes intact, playback identical
- [ ] Bypass toggle mid-playback — no glitch, no state reset on resume

---

## 5. Stress & edge cases

Per §7.7:

- [ ] Trigger spam (30+ in 5s) — no crash, no stuck voice
- [ ] Hold chord → press Panic → next note plays cleanly
- [ ] Load/unload samples 50× consecutively — Instruments → Allocations shows no leak
- [ ] Drag 200MB sample — graceful load (the off-lock decode path in `SampleLoader` should keep `processBlock` from stalling)
- [ ] Drag non-audio file — graceful rejection
- [ ] Load 24-bit / 32-bit float / 96k / 192k source samples — correct resampling
- [ ] Load `.mp3` — decodes, plays, resamples (validates `JUCE_USE_MP3AUDIOFORMAT=1` gate)
- [ ] All 20 slots filled; switch Series ↔ Random mid-playback — no dropout
- [ ] Multi-instance: 8 tracks × 8 instances, all playing — independent state, no crosstalk
- [ ] Editor open during host's offline render — no UI/audio thread deadlock
- [ ] **Mixed-duration pool**: 1 multi-minute MP3 + 5 short WAVs, Random Algorithm tick 15+ (`sampleStartRndPos > 0`) — every sample audible (validates §3c fix from commit `79fb930`)
- [ ] **Drag-reorder slot during heavy trigger spam** — no crash, no UI/audio desync (validates `getCallbackLock()` contract from commit `17637da`)

---

## 6. Performance & sweeps

Per §7.8:

- [ ] Buffer size sweep: 32 / 64 / 128 / 256 / 512 / 1024 — no glitches at any size
- [ ] Sample rate sweep: 44.1 / 48 / 88.2 / 96 / 192 kHz — no glitches, correct pitch, **no tail truncation** (validates `prepareToPlay → rebuildLoadedIndices()` fix from commit `a62b262`)
- [ ] CPU < 0.5% at 64 samples / 48k / 1 voice on M1
- [ ] CPU < 5% at 8 instances all playing
- [ ] RAM < 80MB per instance (10 samples × 2MB each)
- [ ] 30-min continuous playback — RAM stable, CPU stable, no thermal runaway
- [ ] Idle CPU compare: knob-static vs knob-wiggling — confirms `ToneControl` coefficient cache is short-circuiting (the largest single CPU win when idle)

---

## 7. Platform-specific (Mac)

Per §7.9:

- [ ] Apple Silicon native — runs without Rosetta translation indicator in Activity Monitor
- [ ] If Intel Mac available: install universal binary on Intel, verify load + play (no missing-symbol crashes)
- [ ] If macOS 11.0 VM available: load + play on Big Sur (the deployment-target floor)

---

## 8. Code cleanup

- [ ] `DBG()` audit — no debug logs left in prod paths (`processBlock`, `RRVoice::renderNextBlock`, sample-load hot path, repaint loops)
- [ ] Accessibility — `setDescription` and/or `setHelpText` on every control (knobs, dual-thumb sliders, buttons, sample slots, mode toggle, header buttons)
- [ ] Visual regression sanity: every section header (SAMPLE POOL, RANDOM ALGORITHM, AMPLITUDE, TONE, PITCH, SAMPLE START/END) still shows the four-layer gradient/shadow/sheen treatment per CLAUDE.md (don't simplify by accident during cleanup)

---

## 9. Installer scaffolding (unsigned)

The `--sign` flags become one-liners once the Developer ID Installer cert arrives. Build and verify the rest now.

- [ ] Write `pkgbuild` script for each component (VST3, AU, Standalone) with correct install destinations:
  - VST3 → `/Library/Audio/Plug-Ins/VST3/`
  - AU → `/Library/Audio/Plug-Ins/Components/`
  - Standalone → `/Applications/`
- [ ] Write `productbuild` script combining the component pkgs into one distribution `.pkg`
- [ ] Build the unsigned `.pkg`, run on a fresh user account or VM if available — confirm files land at the right paths
- [ ] Write Mac uninstaller shell script (removes all installed files cleanly)
- [ ] Verify uninstaller against an installed copy, then re-install — round-trip clean

---

## 10. Ready-to-fire when Apple approval lands

Don't run these yet — staged so the moment certs land you can ship.

- [ ] `codesign --force --deep --options runtime --sign "Developer ID Application: …" --timestamp` for `.vst3`, `.component`, `.app`
- [ ] Sign the `.pkg` with the Developer ID Installer cert (`productbuild --sign …` or `productsign`)
- [ ] `xcrun notarytool submit "Installer.pkg" --apple-id … --team-id … --password "@keychain:AC_PASSWORD" --wait`
- [ ] `xcrun stapler staple "Installer.pkg"`
- [ ] Gatekeeper test: download stapled `.pkg` from a clean browser session → install → load in Logic without "developer cannot be verified" dialog
- [ ] Back up `.p12` cert exports + app-specific password to password manager

---

## What's deliberately NOT here

- Anything Windows-side (build, signing, SmartScreen, DPI, FL Studio, Reaper-Win) — separate machine
- Code signing / notarization steps (§10 above is *prep* only — execution waits for cert)
- Beta program, KVR listing, press kit, README/CHANGELOG, demo content — non-technical
- AAX work — deferred to v1.1 per spec.md §1.3
- CI workflow (`.github/workflows/build.yml`) — needs Windows runner too, defer until both platforms ready

---

## 11. Deferred to a focused pass before v1.0 tag

### Compiler warning cleanup (186 in `Source/`)

Build succeeds but emits warnings the v1.0 tag should not ship with. Bucket strategy when you tackle:

- **Pro-reserved fields** (`unused-private-field` on envelope/EQ/transient/paired-key members in `RRvoice.h`, `unused-variable` in `TransientShaper.cpp`, etc.) — annotate `[[maybe_unused]]`, **do not delete** (per `update-premium-spec.md` §2: Pro keeps these active).
- **`unused-parameter` in JUCE override signatures** (e.g., `RRVoice::stopNote`, `RRSound::appliesToChannel`) — drop the parameter name in the function signature to silence cleanly. `void stopNote(float, bool)` is valid for the override.
- **`shadow-field-in-constructor` in `SampleLoader.cpp`** (5 occurrences) — rename ctor params with leading `in_` or trailing `_` to break the shadow.
- **`implicit-int-float-conversion`, `sign-conversion`, `sign-compare`, `float-conversion`** (~110 total) — explicit casts (`static_cast<float>(x)`, `static_cast<int>(x)`). **Be careful in `RRvoice.cpp` audio thread paths** — verify no semantic change (e.g., truncation vs rounding).
- **`shadow-uncaptured-local`, `misleading-indentation`** (4 total) — local fixes, low risk.
- **`deprecated-declarations` (6)** — investigate; likely transitive through JUCE includes (`<codecvt>`/`wstring_convert`). If genuinely Apple SDK noise we can't fix, suppress with `target_compile_options(... PRIVATE -Wno-deprecated-declarations)` scoped to JUCE includes only, not our code.

Estimate: 1–2 hours of mechanical edits. Re-run the universal Release build after — gate is zero warnings in `robin-control-redesign/NewProject/Source/`.
