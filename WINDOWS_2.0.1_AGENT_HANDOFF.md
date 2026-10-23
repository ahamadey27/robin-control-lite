# Robin Control Lite 2.0.1 — Windows agent handoff

Prepared October 23, 2026. Read this first on Alex's Windows computer, then
`AGENTS.md`, `WINDOWS_BUILD_AND_AAX.md`, `TESTING.md`, and
`PRO_TOOLS_RELIABILITY_PLAN_2026-10-22.md`. This is a Windows compilation,
validation and packaging task; it is not authorization to publish a release.

## Goal and source

Build **native Windows x64 VST3 and AAX 2.0.1**, with Moonbase enabled, using
the current corrected Lite source. Produce a signed Windows installer after
Windows-specific validation. AU is Mac-only; exclude Standalone from delivery.

Authoritative repository: private `ahamadey27/robin-control-lite`, currently
`main` on this Mac, despite its historical folder name `robin-control-redesign`.
Do not use the retired `round-robin-lite` sibling or premium Robin Control.
Source checkpoint when writing this handoff: **`2a72bcfd973c3422be6b7e6c1c825a16742b7693`**,
which contains the 2.0.1 version changes and Mac acceptance documentation.
This new handoff and subsequent documentation edits must also be transferred
or committed/pushed before another computer can read them through Git. Record
the actual Windows checkout commit; do not assume local Mac files are pushed.

For an existing Windows checkout, inspect branch, status and product-specific
Windows commits before fetching/updating. Preserve local work and any Windows
installer/signing/artwork fixes; do not reset, clean, or overwrite them to match
the Mac. Resolve source differences deliberately. If no checkout exists, clone
`https://github.com/ahamadey27/robin-control-lite.git` through normal Git auth.
Never copy CMake caches, Mac binaries or a Mac `.pkg` to build Windows.

The Mac 2.0.1 package has passed automated validation, signing/notarization and
Alex's reported install/PatchWork retest. This is context, **not Windows proof**.
Company context records public Windows 2.0.0 VST3/AAX; older September documents
that describe all Windows work as pending are historical. Inspect the existing
Windows workflow rather than repeating completed PACE onboarding.

## What changed, and what must survive

1. **F1, build visibility:** CMake explicitly hides C/C++/Objective-C/Objective-C++
   internals for shared code and each format. The Mac export checker catches
   internal JUCE/Moonbase exports and missing host entry points in every slice.
   This targets the customer VST3 editor crash inside PatchWork/Pro Tools:
   invalid free of JUCE's static `emptyString`, with symbol interposition the
   leading diagnosis. Preserve the settings, but Windows exports require PE/
   MSVC tools; the Mach-O checker does not validate Windows DLLs.
2. **F2, pool mutation transactions:** `addPoolSamples`, `replacePoolSample`,
   `deletePoolSample`, `clearSamplePool` and callback-locked helper paths commit
   slots, playback indices and sounds consistently. Decode/resample is staged
   outside the callback lock. Preserve additive loading, rollback on failed
   replacement, sample order, active playback storage and rate-change handling.
   `PoolMutationTransactions` and concurrency tests cover the real APIs.
3. **F3, guarded state parsing:** `Source/Data/StateInput.h` preflights binary XML
   framing, UTF-8, depth 32, structural items 8192, DTD/entities and the existing
   8 MiB byte cap. Host state and `.rrpreset` use the guarded route. Preserve
   parameter IDs and existing valid state; do not replace this with a plain
   `getXmlFromBinary` call. `StateInputPreflight` includes the 50,000-level case.
4. **Metadata:** root/inner CMake and Projucer now say 2.0.1. Runtime sources
   otherwise match the combined diagnostic fix at `65dc245`.

Frozen identity: target `RobinControlLite`, name `Robin Control Lite`, company
`conduit.dsp`, codes `Cdsp` / `rcll`, bundle ID `dsp.conduit.RobinControlLite`.
Keep all parameter IDs, monophonic/any-key behavior, global pitch and preset
format. Moonbase is Lite's own product `robin-control-lite`; retain its 90-day
online grace, permanent offline path and disabled analytics. Do not copy
premium's policies. PACE is AAX signing only, not customer iLok DRM.

## Windows artwork/lifetime prerequisite (F5)

**Do not overlook this before distribution.** This Mac checkout of Lite still
uses a function-static `juce::Image logo` and default-backend keyed/output
images in `PluginEditor.cpp`. The 2.0.1 Mac work did not implement an F5 fix.
Earlier Windows source may contain a platform fix absent here: compare first,
preserve it if present, and record the actual distributed-source provenance.

Robin Control premium supplies an implementation reference: editor-owned
`SharedResourcePointer<BrandImages>` plus `SoftwareImageType` conversion in
its editor files, releasing graphics resources before DLL static teardown.
If Lite still needs that correction, adapt the lifetime/storage pattern while
preserving Lite's approved artwork/layout; do not copy the premium UI wholesale.
Test GUI close, plug-in removal and host exit on Windows. A clean Mac pass does
not establish Windows graphics lifetime safety. Any shared-source edit creates
a new source delta: retain the Mac artifacts and record which platform/build
has actually tested the new code; do not silently reuse earlier source hashes.

F4 synchronous license-store contention and F6 note-on sample copies are
separate follow-up items, not fixed by this update. Do not redesign production
DRM or sample ownership as an incidental compiler workaround.

## Native x64 build

Use Visual Studio 2022 Desktop C++ / v143 x64 and a Windows SDK, CMake 3.22+,
Python 3, JUCE **8.0.15** at
`91ad83ae34a81e0833b1a2b0866f54846370ae53`, and authorized Windows AAX SDK 2.9.0.
Use existing installed tools when suitable. Moonbase 4.4.0 is checksum-pinned;
retain the build-local `MoonbaseHardening.cmake` corrections. Do not weaken
dependency pins to get a build through.

From the repository root in PowerShell, adjust SDK paths to the actual machine:

```powershell
$rclJuce = Join-Path $env:USERPROFILE "SDKs/JUCE"
$rclAax = Join-Path $env:USERPROFILE "SDKs/aax-sdk-2-9-0"
$rclBuild = "NewProject/build-windows-2.0.1"
$rclEvidence = Join-Path "Releases/Testing/2.0.1" ("windows-x64-" + (Get-Date -Format "yyyyMMdd-HHmmss"))
if (Test-Path $rclBuild) { throw "Choose a fresh build directory; preserve the existing cache" }
if (-not (Test-Path "$rclJuce/CMakeLists.txt")) { throw "Set rclJuce to the installed pinned JUCE" }
if (-not (Test-Path "$rclAax/Interfaces/AAX.h")) { throw "Set rclAax to the authorized Windows SDK" }
git status -sb
git rev-parse HEAD
$rclJuceCommit = git -C "$rclJuce" rev-parse HEAD
if ($LASTEXITCODE -ne 0 -or $rclJuceCommit -ne "91ad83ae34a81e0833b1a2b0866f54846370ae53") { throw "JUCE revision mismatch" }
New-Item -ItemType Directory -Path $rclEvidence | Out-Null
Start-Transcript -Path (Join-Path $rclEvidence "build-transcript.txt")

cmake -S NewProject -B "$rclBuild" -G "Visual Studio 17 2022" -A x64 `
  "-DJUCE_PATH=$rclJuce" "-DJUCE_AAX_SDK_PATH=$rclAax" `
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON `
  -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF -DRCL_BUILD_STANDALONE=OFF -DRCL_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) { throw "Configure failed" }

cmake --build "$rclBuild" --config Release --parallel 4 `
  --target RobinControlLite_VST3 RobinControlLite_AAX RobinControlLiteTests RobinControlLiteLicensingTests
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

& "$rclBuild/tests/RobinControlLiteTests_artefacts/Release/RobinControlLiteTests.exe"
if ($LASTEXITCODE -ne 0) { throw "Engine/state/pool tests failed" }
& "$rclBuild/tests/RobinControlLiteLicensingTests_artefacts/Release/RobinControlLiteLicensingTests.exe"
if ($LASTEXITCODE -ne 0) { throw "Synthetic licensing tests failed" }
Stop-Transcript
```

PowerShell continuation backticks must be the last character on the line.
Visual Studio is multi-configuration: `--config Release` selects Release;
`CMAKE_BUILD_TYPE` does not. Explicitly building AAX prevents a missing SDK from
silently omitting it. Inspect JUCE's working tree as well as its commit before
claiming dependency parity. Record source changes and source hashes separately
from a commit when the worktree is dirty.

Outputs are under `$rclBuild/RobinControlLite_artefacts/Release/`:

- `VST3/Robin Control Lite.vst3/` (expected PE binary in `Contents/x86_64-win/`).
- `AAX/Robin Control Lite.aaxplugin/` (expected PE binary in `Contents/x64/`).
- Both must carry `Contents/Resources/RCLBuildConfig.txt` containing
  `version=2.0.1`, `moonbase=ON`, `final=ON`, plus `MoonbaseNotices.txt`.
- AAX must retain `Contents/Resources/RobinControlLitePages.xml`.

Verify the actual output layout. Distribute whole bundles, not just inner PE
files. Inspect x64 architecture, file/product version and exports with MSVC/
Windows tools such as `dumpbin`; confirm host entry points against the installed
SDK. Do not run `scripts/check-macos-exports.py` on PE files or assume CMake's
Unix visibility properties alone establish clean Windows exports.

## Windows signing, validation and installer

Use the established Windows workflow from this machine/repository history.
The Mac checkout does not contain a checked-in Lite `.iss` installer; locate
and reconcile the actual maintained Windows 2.0.0 installer/signing sources.
Do not overwrite them with a Mac script or premium's product identity. If
those inputs are unavailable, report that concrete packaging dependency while
still completing the native compilation and unaffected checks.

1. Run Windows VST3 pluginval **strictness 10 with GUI tests**, unsigned as
   useful and again against the exact signed bundle. Use the verified local
   `pluginval.exe`; require successful Editor, editor-while-processing and
   editor-automation cases and normal process exit. Preserve full logs.
2. Inspect `wraptool.exe` / DigiShell versions and help on this Windows machine.
   `scripts/setup-pace-signing.py` and `scripts/test-aax-native.py` are Mac-specific;
   do not run them unchanged. Reuse established Windows credentials privately.
   No Mac Developer ID certificate string is a Windows signing identity.
3. Run every applicable Windows AAX Native validator test before/after signing,
   with per-test results and exact binary hashes. Discover the installed test
   list; the Mac's 13 checks are context, not a hard-coded Windows result count.
   Record the HDX-only cycle test as N/A only if the Windows toolkit and actual
   Native-only descriptor establish inapplicability. No other failures are waived.
4. Stage new copies and use PACE **`sign`**, never `wrap`, with Lite's existing
   Signing Only Wrap GUID **`8F95C7F0-B538-11F1-8437-00505692AD3E`** and the
   machine's established Windows certificate/signing workflow. Verify PACE names
   Robin Control Lite / Conduit DSP LLC and says **signed, not wrapped**. Verify
   Authenticode/timestamps as required by that workflow. Never mutate signed
   staging afterward or infer signing readiness from an installed SDK alone.
5. Test on native Windows: retail Pro Tools AAX cold scan and mono/stereo MIDI;
   corrected VST3 inside PatchWork; editor cycles and explicit insert removal/
   reinsertion; clean host exit; pool Add/Replace/Delete/Clear during playback;
   Series/Random; session/preset restoration; buffer/rate changes and offline
   bounce. Exercise activation/offline behavior without deleting real licenses
   or changing the machine clock. Record host/wrapper/OS/tool versions and the
   actual loaded PE path/hash. Preserve existing plug-ins before temporary tests.
6. Build the 2.0.1 Windows installer from only verified signed VST3/AAX payloads.
   Preserve user samples/presets/licenses, existing runtime prerequisites and
   uninstall behavior. Check payload versions/build markers/notices/page tables,
   target paths, installer/uninstaller signatures, upgrade and fresh-install
   behavior. No AU or Standalone. A Mac `.pkg` cannot serve as the Windows installer.

Expected system destinations are 64-bit Common Files:

```text
C:\Program Files\Common Files\VST3\Robin Control Lite.vst3
C:\Program Files\Common Files\Avid\Audio\Plug-Ins\Robin Control Lite.aaxplugin
```

Keep logs, raw builds and signed staging in new ignored evidence directories.
Place the finished installer/checksum in the project's established official
installer directory (`Releases/Installers/` on the current Mac handoff), with
an unambiguous Windows filename such as `RobinControlLite-2.0.1-Windows.exe`.
Do not overwrite previous releases. Record the final installer hash, source
revision/diff, SDK/compiler/OS versions, tests, signatures and open manual gates
in tracked Markdown. Existing Windows VST3 CI is useful but does not establish
AAX, installer, F5 graphics lifetime or customer acceptance. Do not trigger extra
hosted jobs or publish/upload merely because compilation succeeds.

## Completion report required from the Windows agent

- Actual source revision and any Windows-only changes, including the F5 outcome.
- Native x64 VST3/AAX output identities, versions, build profiles and hashes.
- Engine/licensing, GUI pluginval, applicable AAX and host results, separating
  automated, operator-reported, failed/inconclusive and not-run checks.
- Exact signed installer path/checksum, payload/signature verification and
  restore/upgrade status; name any concrete remaining dependency.
- No claim that Windows succeeded based on Mac evidence; no release publication
  or changes to Moonbase commerce/activation policy without Alex's instruction.
