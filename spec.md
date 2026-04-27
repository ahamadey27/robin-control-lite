# spec.md — Robin Control Lite (rebrand pending)

> **Status:** Pivot in progress. As of 2026-04-25, this folder is being promoted from "design sandbox" to the **authoritative project** for the free Round Robin sampler plugin. The sibling folder `../round-robin-lite` becomes legacy/reference once this spec is signed off.

This document is the single source of truth for: identity, build, formats, signing, distribution, licensing, testing, and the migration plan. Everything in `CLAUDE.md` that contradicts this file is now stale and needs updating once decisions below are made.

---

## 1. Open decisions

### 1.1 Plugin identity — DECIDED 2026-04-25
The product name is **Robin Control Lite**. The sibling project `../round-robin-lite` is retired (kept on disk as read-only reference; no longer built or installed). Because the sibling never shipped, fresh codes are minted instead of reusing `rrll`/`Rrlt`.

| Field | Value |
|---|---|
| Product name | `Robin Control Lite` |
| CMake target / project | `RobinControlLite` |
| Company name | `conduit.dsp` |
| Plugin code | `rcll` |
| Manufacturer code | `Cdsp` |
| Bundle ID | `dsp.conduit.RobinControlLite` |
| Version | `1.0.0` |

The sibling stays on disk as legacy reference and must not be built side-by-side with this project, since both would otherwise want to install plugins under their respective names but share runtime state via `~/Library/Audio/Plug-Ins/`. Tag the sibling's last commit (`git -C ../round-robin-lite tag legacy-final`) and stop running its CMake.

### 1.2 JUCE license tier (`{{JUCE_LICENSE}}`)
For a free plugin, three legal paths (JUCE 8, current as of Jan 2026):

1. **Personal license — recommended.** Free, no splash screen, no analytics requirement. Permitted for commercial use *and* free distribution while your annual revenue from JUCE-built products stays under JUCE's threshold (currently ~$40k USD/yr — confirm at juce.com/get-juce). Best fit for a free plugin you want to remain closed-source.
2. **GPLv3.** Free, no revenue cap, but your *plugin source must be open-sourced* under GPLv3. Pick this only if you intend to publish the source.
3. **Educational.** Free for students/educators; not appropriate for public distribution.

**Recommendation: Personal.** Confirm the current revenue threshold on JUCE's site at decision time — terms change.

### 1.3 AAX strategy (`{{AAX_PLAN}}`)
AAX is **opt-in extra work**. It is not free even though the plugin is free.

- Avid Developer registration (free): https://developer.avid.com/
- AAX SDK download (free, NDA-style click-through)
- **PACE / iLok signing is mandatory** for Pro Tools to load AAX plugins outside dev mode. Avid offers a free signing program for free plugins, but you submit each release for signing. Build → submit → wait → distribute the signed binary.
- Mac and Windows AAX both require this signing.

**Recommendation: ship VST3 + AU + Standalone in v1.0; add AAX in v1.1 once the rest is stable.** AAX adds ~1–2 weeks of plumbing and a recurring signing step per release.

#### 1.3.1 Avid developer portal — navigation gotchas
The portal is genuinely clunky. Documenting what works so the next setup goes faster:

- The public `developer.avid.com/audio` "Download Evaluation Toolkit" page **shows only a Back button** unless you're signed in as an enrolled developer. The click-through EULA only renders when authenticated.
- After registering, **"My Toolkits and Downloads" starts empty** ("You do not own any products under this account yet"). The AAX toolkit must first be claimed via the **SDK Toolkits** catalog link in the dashboard. Once claimed (free), the SDK + tools list appears in My Toolkits.
- The public AAX page never serves the actual files — every download happens inside the authenticated dashboard.

**Status (2026-04-26):** Avid Developer account active under hamadey@gmail.com; AAX evaluation toolkit claimed; downloads pending. See `memory/project_aax_dev_setup.md`.

#### 1.3.2 Minimal AAX SDK download set (Apple Silicon Mac)
The dashboard lists 50+ items. For getting Robin Control Lite building as AAX, only these are required:

| Item | Size | Why |
|---|---|---|
| **AAX SDK 2.9.0** | 41.28 MB | Headers/libs JUCE links against |
| **AAX Developer Tools Beta 22.R4.0.1 arm64 (Mac)** | 191.79 MB | DigiShell, validator, signing utilities. Use `22.9.0.1 x86_64` on Intel Macs |
| **DigiShell and AAX Validator 24.6 Arm (Mac)** | 298.31 MB | Validates the built `.aaxplugin`. Arch-matched |
| **Pro Tools Developer 2025.12.0 Arm (Mac)** | 2.29 GB | The **Dev** build loads unsigned/eval AAX. Regular Pro Tools 2025.12 rejects unsigned plugins |
| **Evaluation License.pdf** (AAX SDK section) | — | Read before building |

Optional but useful: **AAX Plugin Test Plan (January 2024)** (664 KB) — Avid's official validation checklist for commercial submission.

**Skip:**
- *JUCE to AAX DSP Example* and *Page Table Editor* — AAX DSP is for SHARC chips on HDX hardware; we ship AAX Native only.
- *kTrace / WPR Capture Tools* — only if Avid asks for traces during a support case.
- *HD Driver, Avid Cloud Client Services, Legacy, Sibelius, Pro Tools Demo Session* — unrelated to plugin dev.
- *Pro Tools 2025.12 (non-Dev) and Pro Tools Beta builds* — non-Dev rejects unsigned plugins; Beta is only for forward-compat testing against unreleased PT versions.

#### 1.3.3 v1.1 AAX onboarding order of operations
- [x] Avid Developer account created and AAX evaluation toolkit claimed (2026-04-26)
- [ ] Download the §1.3.2 set; unpack AAX SDK to a stable path (e.g. `~/SDKs/AAX_SDK_2.9.0/`)
- [c] Install Pro Tools Dev (long install)
- [ ] Install AAX Developer Tools (gives DigiShell, validator, signing utilities)
- [ ] Wire `JUCE_AAX_SDK_PATH` into `NewProject/CMakeLists.txt`; add `AAX` to `juce_add_plugin(... FORMATS ...)`
- [ ] Build → load the produced `.aaxplugin` in Pro Tools Dev → smoke test
- [ ] **Order an iLok USB key** (2nd or 3rd gen) from ilok.com — required for commercial signing, not for eval builds. Order early; shipping takes days
- [ ] When ready to ship: email `audiosdk@avid.com` to provision the commercial license + signing tools, then submit `.aaxplugin` for PACE signing per release (see §5.3)

### 1.4 Repository / GitHub identity
- [x] Rename repo from `robin-control-redesign` to `robin-control-lite` (done 2026-04-26; local folder name unchanged)
- [x] Public or private until launch? — **Private** through launch
- [x] License file (MIT/proprietary EULA) — separate from JUCE's license, this covers *your* code (LICENSE = All Rights Reserved, shipped 2026-04-26)

---

## 2. Plugin identity (applied)

The values from §1.1 are now wired into `NewProject/CMakeLists.txt` and `NewProject/RobinControlLite.jucer` (renamed from `robindesign.jucer`). Source-level user-visible strings (`Source/PluginEditor.cpp` header, `Source/PluginEditor.h` About dialog, `Source/PluginProcessor.cpp` init log) already say "Robin Control Lite" — no source edits needed.

Outstanding identity-related cleanup:
- [ ] `README.md` — still describes the project under the original framing; needs a rewrite for end users (see §10 step 6)
- [x] `CLAUDE.md` — drop the "design sandbox / don't touch audio engine" framing (done; see §10 step 5)
- [ ] `NewProject/Builds/` — stale Projucer output trees from before the CMake era; safe to delete

---

## 3. Build system

### 3.1 Authoritative build = CMake
Projucer file is kept in sync but CMake wins. All CI, all release builds go through CMake.

### 3.2 macOS local build
```bash
cd NewProject
cmake -B build -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --config Release
```
Outputs:
- `build/RobinControlLite_artefacts/Release/VST3/Robin Control Lite.vst3`
- `build/RobinControlLite_artefacts/Release/AU/Robin Control Lite.component`
- `build/RobinControlLite_artefacts/Release/Standalone/Robin Control Lite.app`

`COPY_PLUGIN_AFTER_BUILD TRUE` already copies VST3 to `~/Library/Audio/Plug-Ins/VST3` for testing. AU goes to `~/Library/Audio/Plug-Ins/Components`.

### 3.3 Windows local build
On a Windows 10/11 machine with Visual Studio 2022 (Desktop C++ workload):
```powershell
cd NewProject
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
Outputs:
- `build\RobinControlLite_artefacts\Release\VST3\Robin Control Lite.vst3`
- `build\RobinControlLite_artefacts\Release\Standalone\Robin Control Lite.exe`

Default install location for VST3 on Windows: `C:\Program Files\Common Files\VST3\`.

### 3.4 Building Windows from a Mac — don't try
JUCE does not cleanly cross-compile Mac→Windows. Realistic options, ranked:
1. **GitHub Actions CI** with a `windows-latest` runner (free for public repos, ~$0.008/min for private). Recommended — see §6.
2. **Parallels Desktop / VMware Fusion** with Windows 11 + VS2022. Local but slow.
3. **A physical Windows box** on the network. Fastest iteration but extra hardware.

### 3.5 JUCE pinning — DONE 2026-04-25
`NewProject/CMakeLists.txt` now resolves JUCE in two stages: it uses a local checkout if `JUCE_PATH` (default `~/Documents/JUCE`) exists, otherwise it falls back to `FetchContent` against `juce-framework/JUCE` tag `8.0.4`. This keeps the local dev path fast while making CI / fresh clones build out of the box. Override per invocation with `cmake -DJUCE_PATH=/path/to/JUCE -B build`.

### 3.6 Universal binary (Mac)
`-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"` produces a fat binary. Required for Apple Silicon native + Intel compatibility. Adds build time but is non-negotiable for distribution in 2026.

### 3.7 Minimum OS targets
- macOS: 11.0 (Big Sur). 13.0 currently in CMakeLists is too aggressive — drop to 11.0 to cover Apple Silicon's first OS plus Intel users on older machines.
- Windows: Windows 10 1809+ (the JUCE 8 floor).

---

## 4. Plugin formats

| Format | Mac | Windows | v1.0 ship? | Notes |
|---|---|---|---|---|
| VST3 | ✅ | ✅ | ✅ | `.vst3` bundle (Mac) / folder (Windows). Steinberg SDK ships with JUCE. |
| AU | ✅ | ❌ | ✅ | `.component`. Mac-only, required for Logic/GarageBand. |
| Standalone | ✅ | ✅ | ✅ | `.app` / `.exe`. Useful for testing without a host. |
| AAX | ✅ | ✅ | ❌ (defer to v1.1) | Pro Tools only. Requires PACE signing — see §1.3. |
| VST2 | — | — | ❌ | Deprecated by Steinberg. Don't ship. |
| LV2 | ✅ | ✅ | ❌ | Linux-leaning ecosystem, low ROI for v1. |
| CLAP | ✅ | ✅ | optional | Newer open standard. Bitwig/Reaper support. Cheap to add (JUCE 8 supports it via `clap-juce-extensions`). Consider for v1.1. |

In `juce_add_plugin(...)` the `FORMATS` line becomes:
```cmake
FORMATS  VST3 AU Standalone   # v1.0
# FORMATS  VST3 AU AAX Standalone   # v1.1
```

---

## 5. Code signing, notarization, distribution

### 5.1 macOS
**Required.** Without signing+notarization, modern macOS quarantines the plugin and hosts won't load it cleanly.

- **Apple Developer Program:** $99/yr. Get a "Developer ID Application" certificate (for `.vst3`/`.component`/`.app`) and "Developer ID Installer" certificate (for the `.pkg` installer).
- **Sign every artifact:**
  ```bash
  codesign --force --deep --options runtime \
    --sign "Developer ID Application: Your Name (TEAMID)" \
    --timestamp \
    "Robin Control Lite.vst3"
  ```
  Repeat for `.component` and `.app`.
- **Build a `.pkg` installer** with `pkgbuild` + `productbuild`; sign with the Installer cert.
- **Notarize:**
  ```bash
  xcrun notarytool submit "Installer.pkg" \
    --apple-id hello@conduitdsp.com \
    --team-id TEAMID \
    --password "@keychain:AC_PASSWORD" \
    --wait
  xcrun stapler staple "Installer.pkg"
  ```
- Hardened runtime + secure timestamp are required for notarization; both are set above.

### 5.2 Windows
**Strongly recommended.** Without signing, SmartScreen warns users; some hosts (and corporate antivirus) refuse to load.

- **Code signing certificate:** ~$200–500/yr (Sectigo, DigiCert, SSL.com). EV certs avoid SmartScreen "unknown publisher" warnings instantly; standard OV certs accumulate reputation over time.
- **Sign:**
  ```powershell
  signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 \
    /a "Robin Control Lite.vst3"
  ```
- **Installer:** Inno Setup (free) or NSIS. Sign the installer `.exe` too.
- **Cheaper alternative for v1.0:** ship unsigned and document the SmartScreen workaround in README. Not great UX but acceptable for a free product's first release.

### 5.3 AAX (when added)
PACE signing replaces standard signing for AAX. Submit unsigned `.aaxplugin` to Avid; they return a signed copy. Process documented in the AAX SDK.

### 5.4 Distribution channels
- Project website (conduit.dsp domain) with direct download
- KVR Audio listing (free, ~24h moderation)
- Plugin Boutique (free plugins accepted, takes a small cut even on free? confirm their current terms)
- GitHub Releases — fine for technical users

---

## 6. CI/CD (GitHub Actions)

Add `.github/workflows/build.yml` covering:
1. **Pull request:** build Mac universal + Windows x64 in Release, run pluginval (see §7), upload artifacts.
2. **Tag push (`v*.*.*`):** full release pipeline — sign, notarize, build installers, attach to GitHub Release.

Matrix:
```yaml
matrix:
  include:
    - os: macos-14   # ARM runner, builds universal
    - os: windows-2022
```

Secrets required (GitHub Settings → Secrets):
- `MAC_DEVELOPER_ID_CERT_P12` (base64), `MAC_CERT_PASSWORD`
- `MAC_NOTARY_APPLE_ID`, `MAC_NOTARY_TEAM_ID`, `MAC_NOTARY_PASSWORD` (app-specific password)
- `WIN_CODESIGN_CERT_PFX` (base64), `WIN_CODESIGN_PASSWORD`

This is a v1.1 nice-to-have. v1.0 can ship from a local Mac + a Parallels Windows VM.

---

## 7. Testing

The release gate is **all of §7.2 (automated) + §7.4 (per-host smoke) on every Tier 1 host + §7.11 (master checklist) clean**. Anything below "Tier 1" is best-effort; bugs there don't block release but should be tracked.

### 7.1 Test pyramid
| Layer | Catches | Speed | Run when |
|---|---|---|---|
| Unit tests | Logic regressions in `RandomizationEngine`, `MidiMapper`, sample resampling | <1s | Every commit |
| pluginval (low strictness) | Crashes, parameter range bugs, basic state recall | ~30s | Every PR |
| pluginval (strictness 10) | Threading bugs, fuzz state recall, edge-case automation | ~5–10 min | Pre-release tag |
| auval | AU-format conformance (Logic refuses AU that fails) | ~10s | Pre-release tag, Mac only |
| Per-host smoke (§7.4) | Real-world host quirks — scan, automation, save/recall | ~5 min/host | Pre-release tag |
| Stress + perf (§7.7–7.8) | Memory leaks, CPU spikes, multi-instance failures | 30+ min | Pre-release tag |
| Beta (§7.10) | Workflows we never thought of | days | Before public launch |

### 7.2 Automated checks

**pluginval** — Tracktion's free fuzzer (https://github.com/Tracktion/pluginval):
```bash
pluginval --strictness-level 10 --validate-in-process --timeout-ms 600000 \
  "build/RobinControlLite_artefacts/Release/VST3/Robin Control Lite.vst3"
pluginval --strictness-level 10 --validate-in-process --timeout-ms 600000 \
  "build/RobinControlLite_artefacts/Release/AU/Robin Control Lite.component"
```
Run at strictness 5 in PR builds (CI), 10 before any release tag. Strictness 10 includes randomized parameter sweeps and state save/recall fuzzing — most state-recall bugs surface here, not in DAWs.

**auval** (macOS, AU only):
```bash
auval -v aumu rcll Cdsp
```
Must pass; Logic refuses AUs that don't.

**Unit tests** — `juce::UnitTest` framework or Catch2. Wire into CMake with `juce_add_console_app` for a separate test binary. Cover:
- `RandomizationEngine` — symmetric/asymmetric ranges, edge values, seeding determinism
- `MidiMapper` — every key pair → MIDI note, root-note offset
- `SampleLoader` — stereo→mono fold, resampling at 44.1/48/96/192k against golden buffers
- APVTS state round-trip — `getStateInformation` → `setStateInformation` reproduces every param

DSP and UI aren't worth unit testing; manual + pluginval covers them.

### 7.3 DAW host matrix

**Tier 1 — must pass before every release.** Covers ~85% of users.

| Host | macOS | Windows | Format(s) tested | Why it's here |
|---|---|---|---|---|
| Logic Pro | ✅ | — | AU | Only DAW that runs `auval`; strictest AU host |
| Ableton Live 12 | ✅ | ✅ | VST3 + AU (Mac) | Largest user base; aggressive plugin scanning |
| Reaper 7 | ✅ | ✅ | VST3 + AU (Mac) | Cheap to license, scriptable, great repro tool |
| FL Studio 21 | — | ✅ | VST3 | Windows-only flagship; quirky automation handling |
| JUCE AudioPluginHost | ✅ | ✅ | VST3 + AU (Mac) | Reference host; isolates host bugs from plugin bugs |

**Tier 2 — best-effort, regression-test only on major releases.**
| Host | macOS | Windows | Notes |
|---|---|---|---|
| Cubase 13 | ✅ | ✅ | Steinberg-canonical VST3 host |
| Studio One 6 | ✅ | ✅ | Tight VST3 + AU coverage |
| Bitwig Studio | ✅ | ✅ | Adds CLAP path once we ship CLAP |
| GarageBand | ✅ | — | AU; sanity check vs Logic |
| MainStage | ✅ | — | AU live-performance host |
| Standalone (`.app`/`.exe`) | ✅ | ✅ | Verify no host = no host-dependent assumptions |

**Tier 3 — only when AAX ships (v1.1+).**
| Host | macOS | Windows | Notes |
|---|---|---|---|
| Pro Tools 2024 | ✅ | ✅ | AAX-only; PACE-signed builds only |

### 7.4 Per-host smoke test (run on every Tier 1 host)

For each host × format combination, walk this checklist. Allow ~5–7 minutes per host once you're warmed up.

**Discovery & load**
- [ ] Plugin appears in host's plugin list after rescan (no validation errors logged)
- [ ] Plugin loads onto a track without warning dialogs
- [ ] Plugin name shown in host matches "Robin Control Lite" (not the bundle name)
- [ ] Manufacturer shown as "conduit.dsp"

**Basic playback**
- [ ] MIDI keyboard (host's virtual or hardware) triggers samples on the expected MIDI notes
- [ ] All 10 sample slots load valid WAVs via the file picker AND drag-drop from desktop
- [ ] Series mode (round-robin) cycles through samples in order
- [ ] Random mode never repeats until pool exhausted (Fisher-Yates intent)
- [ ] Panic button stops all voices immediately
- [ ] Trigger button fires the next round-robin sample without MIDI input

**UI**
- [ ] Editor opens, all controls visible, no clipped layout
- [ ] Open → close → reopen editor — state preserved, no graphical artifacts
- [ ] Resize the host's plugin window if the host allows it — no smeared/stuck rendering
- [ ] About dialog shows correct version, plugin name, conduit.dsp brand

**State save/recall (intra-DAW)** — the highest-value test
- [ ] Load samples + tweak ALL knobs + set ALL randomization ranges
- [ ] Save host project, close host, reopen project
- [ ] Verify: every loaded sample is back in its slot, every param value matches, every randomization range matches
- [ ] Bonus: rename the project file, move it to another folder, reopen — sample paths still resolve (or fail gracefully if absolute paths)

### 7.5 Automation testing (every Tier 1 host)

Automation is where most state-recall and host-integration bugs hide. Run this on at least Logic, Live, and Reaper.

- [ ] Right-click any knob → host's automation menu appears, "Show in automation lane" / equivalent works
- [ ] Record automation on **Volume**: write → playback exactly recreates the moves
- [ ] Record automation on a **randomization-range knob**: writes correctly (these are hidden APVTS sliders driven by the dual-thumb UI — confirm both thumbs automate independently)
- [ ] Record automation on **Sample Start/End** percentages
- [ ] Edit an automation point in the host's lane editor → plugin reflects the new value live
- [ ] Save project → reopen → automation lanes intact, playback identical
- [ ] Host-side parameter modulation (LFOs in Bitwig/Live) — confirm parameter range mapping is sane (0–1 normalized maps to expected min–max)
- [ ] Automation read mode "Off" / "Read" / "Write" / "Touch" / "Latch" all behave per host convention
- [ ] Automating the **same param at the same time** from host + UI thumb — last-writer-wins, no deadlock or glitch
- [ ] Bypass the plugin from host transport — bypass on/off doesn't reset state, no audio glitch on resume

### 7.6 State portability across formats and hosts

Within reason — saving a Live project then opening it in Logic isn't a thing, but these *are* testable:

- [ ] **VST3 ↔ AU parity** (Mac): load same samples, set identical params in both formats, render same MIDI clip → bit-identical (or sub-LSB) output
- [ ] **Plugin's own preset save/load**: save via the plugin chrome (host-agnostic), reload — exact state restored. Confirm preset file is portable: copy the preset file from Mac to Windows, load — works.
- [ ] **Host preset format**: save host-side preset (e.g. Logic's `.cst`, Live's `.adv`) → reload in same host on a different machine — works, samples re-resolved
- [ ] **Plugin scan after move**: install plugin to default location → host scans → move plugin to a custom path → rescan → host re-finds it
- [ ] **Project portability across DAW versions**: save in Live 12.0, open in Live 12.x latest — works (we don't control this but should at least not crash on old/new host versions)
- [ ] **Roundtrip with offline render**: bounce a track to audio → import bounce → matches realtime playback (catches block-size-dependent state bugs)

### 7.7 Stress and edge cases

- [ ] Spam the Trigger button 30+ times in 5 seconds — no crash, no stuck voice
- [ ] Hold a chord while pressing Panic — all voices stop, next note plays cleanly
- [ ] Load and unload samples 50× in a row — no leaks (verify with Instruments → Allocations on Mac, or Application Verifier on Windows)
- [ ] Drag a 200MB sample file → graceful (load progress or rejection, not a hang)
- [ ] Drag a non-audio file → graceful rejection
- [ ] Drag a 24-bit / 32-bit float / 96kHz / 192kHz sample — handled correctly (resampling expected)
- [ ] All 20 slots filled, switch playback mode mid-playback — no dropout
- [ ] Rapidly toggle Bypass during playback — no glitch
- [ ] Save project mid-playback (some hosts allow this) — no corruption on reload
- [ ] **Multi-instance**: 8 instances on 8 tracks, all playing — no crosstalk, independent state
- [ ] **Plugin in a track inside a Group/Bus/Send chain** — works
- [ ] Editor open while host is rendering offline — no UI thread / audio thread deadlock

### 7.8 Performance

| Metric | Target | How to measure |
|---|---|---|
| CPU @ 64 samples / 48k, 1 voice | < 0.5% on M1 / i5-10th gen | Host's CPU meter; `top` for standalone |
| CPU @ 8 instances, all playing | < 5% total | Same |
| RAM per instance (after loading 10 samples × 2MB each) | < 80MB | Activity Monitor / Task Manager |
| First-note latency (UI button → audio out) | < 5ms at 64-sample buffer | Loopback recording |
| Scan time (host's plugin scanner) | < 2s | Stopwatch on cold rescan |

Sweep tests:
- [ ] Buffer sizes: 32, 64, 128, 256, 512, 1024 — no audio glitches at any size
- [ ] Sample rates: 44.1, 48, 88.2, 96, 192 kHz — no glitches; sample playback pitch is correct (resampler not introducing detune)
- [ ] 30-min continuous playback session — RAM stable (no leak), CPU stable (no thermal-related runaway)

### 7.9 Platform-specific tests

**macOS**
- [ ] Apple Silicon native — runs without Rosetta translation indicator
- [ ] Intel via Rosetta — universal binary works on Intel host
- [ ] Run on the lowest supported macOS (11.0 Big Sur) — loads, plays, no missing-symbol crashes
- [ ] AU validation: `auval -v aumu rcll Cdsp` passes with no warnings
- [ ] Gatekeeper / quarantine: download installer from a clean browser → install → load in Logic without "developer cannot be verified" dialog (proves notarization stapled correctly)

**Windows**
- [ ] DPI scaling: 100%, 125%, 150%, 200% — UI legible, controls hit-targets correct
- [ ] Multi-monitor: drag plugin window across monitors with different DPI — no crash, redraws cleanly
- [ ] Run on Windows 10 1809 (the floor) and Windows 11 latest
- [ ] SmartScreen: download installer from clean browser → installs without "Unknown publisher" block (proves code-signing reputation OK)
- [ ] ASIO + WASAPI + WDM driver paths in Standalone — all produce audio

### 7.10 Beta program

Before public launch:
- 8–12 testers covering DAW diversity (≥1 Logic, ≥1 Live, ≥1 Reaper, ≥1 FL, ≥1 Cubase user; ideally a mix of Mac + Windows)
- 2-week minimum window
- Provide a structured feedback form: host + version, OS + version, what they tried, what broke, repro steps
- Track issues in GitHub Issues with `beta` label
- Cut at least one `1.0.0-rc2` from beta feedback before public

### 7.11 Master pre-release checklist

Don't tag `v1.0.0` until **all** of these are green.

**Build artifacts**
- [ ] CMake configures cleanly on a fresh clone (no `JUCE_PATH` env, FetchContent path works)
- [ ] Mac universal binary (`x86_64;arm64`) builds without warnings
- [ ] Windows x64 build via VS2022 succeeds without warnings
- [ ] Version bumped in both `project(... VERSION)` and `juce_add_plugin(... VERSION)`
- [ ] CHANGELOG.md updated for this version

**Automated**
- [ ] pluginval strictness 10 passes on VST3 (Mac + Win)
- [ ] pluginval strictness 10 passes on AU (Mac)
- [ ] auval passes (Mac)
- [ ] Unit tests pass (when added)

**Per-host (Tier 1)**
- [ ] Logic — full §7.4 + §7.5 + §7.6 checklist
- [ ] Live — full §7.4 + §7.5 + §7.6 checklist (both VST3 and AU on Mac)
- [ ] Reaper — full §7.4 + §7.5 checklist (both VST3 and AU on Mac, VST3 on Win)
- [ ] FL Studio — full §7.4 + §7.5 checklist (Windows only)
- [ ] AudioPluginHost — full §7.4 (sanity check that bugs reproduce in a neutral host)

**Stress / perf / platform**
- [ ] §7.7 stress checklist clean
- [ ] §7.8 performance targets met
- [ ] §7.9 platform-specific items clean

**Distribution**
- [ ] Mac: signed (Developer ID Application + Installer), notarized, stapled
- [ ] Mac: clean install on a Mac that's never had the plugin before — works
- [ ] Win: signed (or SmartScreen workaround documented in README)
- [ ] Win: clean install on a fresh Windows VM — works
- [ ] Uninstaller (Mac script + Win uninstaller) removes all installed files

**Sanity**
- [ ] About dialog version matches the git tag
- [ ] No debug logs left enabled (`DBG()` calls in prod paths — review before release)
- [ ] LICENSE + EULA files present and correct
- [ ] README install instructions accurate for both platforms

### 7.12 Bug triage rubric (during testing)

When a tester reports something:
- **Blocker** — crash, audio dropouts >5ms, state-recall failure, can't load → fix before release
- **Major** — UI artifact, host-specific glitch, automation quirk → fix if low-risk; document as known issue otherwise
- **Minor** — cosmetic, edge-case workflow → file for v1.x patch
- **Wontfix** — host bug we can't work around → document in README "Known limitations"

Keep one Tier-1 host running locally during fix iteration so you don't fix something in Logic and re-break it in Live without noticing.

---

## 8. Versioning & releases

- **Semver.** `1.0.0` first stable. Patch for bugfix-only. Minor for new features without breaking presets. Major if APVTS state format changes incompatibly.
- **Bump in two places:** `CMakeLists.txt` `project(... VERSION x.y.z)` and `juce_add_plugin(... VERSION x.y.z)`.
- **CHANGELOG.md** — mentioned in README but doesn't exist yet. Create one. Keep-a-Changelog format.
- **Git tags** drive release builds: `git tag -a v1.0.0 -m "1.0.0"; git push --tags`.
- **Preset-format compatibility:** APVTS param IDs are frozen (per CLAUDE.md). Don't rename any across versions. If you must rename, ship a migration path in `getStateInformation`/`setStateInformation`.

---

## 9. Folder structure (post-pivot)

```
robin-control-redesign/                # (renamed eventually to robin-control-lite)
├── spec.md                            # this file — source of truth
├── CLAUDE.md                          # rewrite once decisions in §1 are made
├── README.md                          # rewrite for end users (not just devs)
├── CHANGELOG.md                       # NEW — create at first release
├── LICENSE                            # NEW — your code's license (separate from JUCE's)
├── EULA.md                            # NEW — end-user terms for the free plugin
├── .github/
│   └── workflows/
│       └── build.yml                  # NEW — CI (v1.1)
├── NewProject/
│   ├── CMakeLists.txt                 # update: name, JUCE pinning, formats
│   ├── RobinControlLite.jucer        # rename
│   └── Source/                        # unchanged structure
├── Releases/                          # NEW — built/signed artifacts (gitignored)
│   ├── macOS/                         # .vst3, .component, .app, .pkg per version
│   ├── Windows/                       # .vst3, .exe, installer .exe per version
│   └── Installers/                    # final signed installers ready to upload
├── Images/                            # design refs (S612, HiFi, Sampler)
├── ImageReferences/                   # additional refs
├── LOGOS/                             # brand assets (FULLLOGO-*)
├── docs/                              # NEW — user-facing docs
│   ├── user-guide.md
│   └── install-mac.md / install-windows.md
└── design-spec.md, RRLite_spec_v2_OLD.md   # archive or delete
```

Add to `.gitignore`:
```
Releases/macOS/*
Releases/Windows/*
Releases/Installers/*
!Releases/**/.gitkeep
NewProject/build/
NewProject/Builds/
```

---

## 10. Migration plan (sandbox → main)

The hard part isn't the spec — it's untangling the relationship with `../round-robin-lite`. Steps, in order:

- [x] **Decide the cutoff.** Either (a) this folder is now main and `round-robin-lite` is archived/deleted, or (b) `round-robin-lite` is archived but kept as read-only reference. Picked: (b), sibling retired as legacy reference.
- [x] **Pick name + JUCE license + AAX plan** (all of §1). Name = Robin Control Lite; AAX deferred to v1.1; JUCE license recommendation = Personal (final confirmation pending).
- [x] **Rename the target.** `robindesign` → `RobinControlLite` everywhere: CMakeLists, .jucer, source-string identity.
- [x] **Fix the JUCE path.** Switch from absolute path to FetchContent or submodule (§3.5). Done — local-checkout-then-FetchContent fallback.
- [x] **Update CLAUDE.md.** Strip the "design sandbox / don't move controls" framing. The redesign constraint is gone — the project is now full-stack development.
- [ ] **Update README.md** for end users, not just developers. Pull learning-resource and proprietary-license language out.
- [ ] **First Mac release dry-run.** Build → sign → notarize → install in Logic and Reaper → verify state recall. Catch process bugs before v1.0 pressure.
- [ ] **Spin up Windows build environment.** Parallels VM is fastest path. Build, install in Reaper-Windows, smoke test.
- [ ] **Tag `v1.0.0-rc1`.** Iterate on the checklist in §7.5 until clean.
- [ ] **Tag `v1.0.0` and ship.**

---

## 11. Things you might be missing

In rough priority order:

- [ ] **Trademark search** for the chosen name. USPTO TESS (US), EUIPO eSearch (EU). 30 minutes. Do this before printing anything.
- [ ] **Domain.** `conduit.dsp` exists; do you also want `robin-control-lite.com` or a `/robin-control-lite` subpath?
- [x] **Privacy policy + EULA.** Even a free plugin needs both if you have a download form, mailing list, or any analytics. EULA v1.0 effective 2026-04-26; Privacy.md pointer ships with repo, canonical at conduitdsp.com/privacy-policy/.
- [ ] **Crash reporting.** JUCE's `juce::SystemStats` + a tiny log file is enough for v1. Sentry / Bugsnag are overkill for a free plugin.
- [ ] **Update mechanism.** Don't build one for v1. Email + a "check for updates" link in the About dialog is fine.
- [ ] **Mailing list / launch list.** Capture emails on the download page. MailerLite is the chosen tool (already on conduitdsp.com).
- [ ] **Documentation site.** A static page per platform (install instructions) goes a long way. README is for GitHub readers; users need a real site.
- [ ] **Demo content / preset pack.** Free plugin sells itself harder if it ships with 5–10 great-sounding sample sets. Footstep packs (the original use case) are a natural starter.
- [ ] **Accessibility.** JUCE 8 added accessibility hooks. At minimum, label every control with `setDescription`/`setHelpText`. VoiceOver/Narrator users will thank you.
- [ ] **Telemetry — explicit "no".** State in the EULA that the plugin doesn't phone home. Free + privacy-respecting is a strong combination.
- [ ] **Support channel.** A Discord server or just `hello@conduitdsp.com` forwarded to your inbox. One bug-report email beats no channel.
- [ ] **Press kit.** Logo PNGs (1x/2x), screenshots (1400×400 + scaled), 50/100/200-word descriptions. Saves hours when sites/blogs ask.
- [ ] **Beta program.** Ship `v1.0.0-rc1` to ~10 trusted testers (DAW diversity matters more than count) for two weeks before public launch.
- [ ] **Legal: VST3 trademark.** "VST" is Steinberg's. The license you accept with the SDK requires specific attribution language — re-read it before the launch page is written.
- [ ] **Backup strategy for signing certificates.** Lose the Mac Developer ID cert and you can revoke + reissue, but lose the Windows code signing cert + private key and you may need to repurchase. Back up the `.p12`/`.pfx` files to a password manager.
- [ ] **Universal Binary 2 sanity check.** Test the AU on an Intel Mac if possible — Apple Silicon-native testing alone has missed bugs that only surface on x86_64.
- [ ] **The two stale folders.** `NewProject/Builds/MacOSX/` and `NewProject/Builds/VisualStudio2026/` are old Projucer output. Decide: delete (recommended, CMake is authoritative) or keep as a fallback path.
- [ ] **`design-spec.md` and `RRLite_spec_v2_OLD.md`** — archive both into `docs/archive/` or delete. Having two stale specs alongside this new one will cause confusion in three months.

---

## 12. Open questions for you

- [x] Pick a name from §1.1 (or propose another). → Robin Control Lite
- [ ] Confirm Personal JUCE license is the path (§1.2).
- [x] Confirm AAX deferred to v1.1 (§1.3).
- [x] Public or private GitHub repo until launch (§1.4)? → Private
- [ ] Do you have an Apple Developer account already, or is that a new $99 expense?
- [ ] Any preference on Mac installer (`.pkg` vs `.dmg`)? `.pkg` is more "professional installer," `.dmg` is "drag to Applications."
- [ ] Target release date for v1.0 — drives how much of the §11 list ships day one vs day 30.

Once §1.1–1.3 are answered, I'll do the rename pass + JUCE pinning + CLAUDE.md rewrite as a single coordinated change.
