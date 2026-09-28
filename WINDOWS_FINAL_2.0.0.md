# Windows final 2.0.0 — September 28, 2026

Local Windows x64 release preparation. Moonbase and the final-release guard
are enabled. The later AAX + VST3 build is recorded below; the earlier VST3-only
artifacts remain preserved. Nothing has been published.
See `FINAL_RELEASE_2.0.0.md` for the separate macOS release and launch checklist.

## Source and build

Plugin source: local commit `1265b95` plus the editor image-lifetime fix described
below. At build time GitHub `main` was
`33b439871b495230bfee7ad03f3dbd065f8d597a`; the local branch was six commits ahead.
Do not identify this binary as a build of the then-current remote main.
Installer/signing scripts were also updated in the working tree.

Evidence directory (ignored by Git):
`Releases/Testing/2.0.0/windows-final-20260928/`.
`build-manifest-r2.json` records the full source commit and corrected source
hashes. `source-r2.patch` preserves the uncommitted source changes.

Build directory: `NewProject/build-windows-final-2.0.0`.
Visual Studio 2022, MSVC 19.43.34810, x64 host/target, Windows SDK 10.0.22621.0.
JUCE 8.0.15 and Moonbase 4.4.0 used the repository's pinned downloads and
Moonbase hardening. Both test targets and the VST3 built successfully in Release.

```powershell
cmake -S NewProject -B NewProject/build-windows-final-2.0.0 `
  -G 'Visual Studio 17 2022' -A x64 -T host=x64 `
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON `
  -DRCL_BUILD_STANDALONE=OFF -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF `
  -DRCL_BUILD_TESTS=ON
$env:CL='/MP1'
cmake --build NewProject/build-windows-final-2.0.0 --config Release `
  --target RobinControlLite_VST3 RobinControlLiteTests RobinControlLiteLicensingTests `
  --parallel 1 -- /p:PreferredToolArchitecture=x64 /p:MultiProcessorCompilation=false
```

The initial invocation supplied absent local JUCE/AAX paths; CMake correctly
fell back to pinned JUCE and omitted AAX. No Windows AAX result is claimed here.
If this terminal exposes both `Path` and `PATH`, normalize them to one process
variable before CMake/MSBuild to avoid duplicate environment-key errors.

Raw bundle:
`NewProject/build-windows-final-2.0.0/RobinControlLite_artefacts/Release/VST3/Robin Control Lite.vst3`.
Its resources contain `version=2.0.0`, `moonbase=ON`, `final=ON`,
`moonbase_sdk=4.4.0-rcl-hardening-1`, and `MoonbaseNotices.txt`.
Unsigned inner PE SHA-256:
`FFC9BAC4C7468DAF82188AAB7BF91B10ACC3EA1998A4CA535C3BB0CD1C08DA33`.

## Windows unload correction

Both released pluginval 1.0.4 and a diagnostic build of pluginval commit
`4c5adc2c1a9910251667152166139a0c37b953e6` reported successful tests but hung
after GUI validation of the initial candidate. Headless validation exited 0.
Noninvasive Microsoft debugger call stacks showed the plugin unloading with
static destruction waiting inside NVIDIA/Direct3D cleanup under the loader lock.
Evidence: `pluginval-current-stacks.log`.

The processed footer logo was a function-static native `juce::Image` that
outlived normal editor/JUCE teardown. It now belongs to each editor, and its
pixel-processing images use software storage. The logo algorithm and artwork
are preserved. Released pluginval now passes GUI checks and exits normally for
both the rebuilt unsigned and Azure-signed candidates. No graphics driver or
system settings were changed.

The first candidate's `signed/` and `installer/` directories are superseded,
retained for diagnosis, and must not be distributed. Use the `*-r2` artifacts.

## Signing and packaging

Device-code Azure CLI login succeeded as `hello@conduitdsp.com`, tenant
`42947a0d-277d-4224-aa0a-da7f6b4c78ce`, subscription
`b916983a-72e5-4ee6-954e-2c1852cd04d6`.
Endpoint `https://eus.codesigning.azure.net/`, account `conduitdspsigning`,
profile `conduitdsp-public-trust`. Credentials remain in the normal Azure CLI
cache outside the repository; no client secret or private certificate was used.

Tools: portable Azure CLI 2.90.0, Microsoft.ArtifactSigning.Client 1.0.128,
x64 SignTool product version 10.0.22621.3233, .NET 8.
`scripts/sign-windows-artifact.ps1` signs with SHA-256 and Microsoft's RFC 3161
timestamp service, then checks SignTool and PowerShell trust, publisher and
timestamp. The metadata file selects Azure CLI credentials only.

Signed staging: evidence directory `signed-r2/Robin Control Lite.vst3/`.
The complete bundle was copied before signing; the raw build was preserved.
The inner PE verified as `Valid`, subject
`CN=CONDUIT DSP LLC, O=CONDUIT DSP LLC, L=Kingston, S=New York, C=US`, with a
Microsoft timestamp. See `sign-vst3-r2.log`.
Signed inner PE SHA-256:
`6841F47093E25DB527641DA2DEC02BDC085222B289060F5CF4C977F8E385D0B7`.

`installer/windows/build-final.ps1` requires explicit signed staging, checks
final build markers and notices, validates the VST3 signature, and refuses to
overwrite an installer. It invokes Inno Setup with the Azure signing script for
both setup and uninstaller. The stable product AppId is preserved.
The historical beta installer script is separate.

Signed installer: evidence directory
`installer-r2/RobinControlLite-2.0.0-Windows.exe`.
An identical copy is ready at
`Releases/Installers/RobinControlLite-2.0.0-Windows.exe`.
SHA-256: `D0132904FFC87C4F67336756E88BF3A3C67CD436723DE0CBF09EA85331E7315B`.
Setup signature verification passed. Build/signing log: `installer-build-r2.log`.
The guard correctly rejected the existing beta bundle with `final=OFF`.

## Validation and remaining work

- Engine/processor regression executable: PASS, exit 0 (`engine-tests-r2.log`).
- Moonbase licensing executable: PASS, exit 0 (`licensing-tests-r2.log`). Uses
  synthetic licenses; covers production grace configuration, invalid claims,
  actual processor MIDI/Trigger/audition gating and async/offline lifecycle.
- Released pluginval 1.0.4, strictness 10, GUI included: PASS, exit 0 before and
  after signing (`pluginval-r2-unsigned-*`, `pluginval-r2-signed-*`). The initial
  candidate's failed shutdown is retained separately as diagnostic evidence.
- Installer and generated uninstaller signing/verification: PASS in the Inno
  build (Inno Setup 6.7.1). The final installer copy matches the tested package.
- Installation/payload verification: PASS after the user ran setup manually.
  All four staged bundle files match the installed files (recorded in
  `installed-payload-comparison.json`). The installed VST3 and `unins000.exe`
  both verify as Valid, timestamped, CONDUIT DSP LLC. The earlier automated
  elevation attempt was canceled; its wrapper exit 0 was not an installer
  result. Existing 1.0.1 uninstall files/registration metadata were preserved
  in `before-install/`; no old VST3 bundle was present before installation.
- Licensed playback/session/offline activation in real DAWs: pending user tests.
- Windows AAX, retail Pro Tools, public upload and fulfillment changes remain
  separate unfinished release checkpoints. Nothing was published.

To finish locally, perform real-host activation/playback/session checks. The
user has installed the package; file integrity and installed signatures have
been verified. Activation can use the existing customer entitlement through
Moonbase's hosted portal before the new website storefront is deployed.
The source correction and packaging scripts
remain local working-tree changes; no commit, push, cloud workflow or upload was
performed by this session.

## Final AAX + VST3 build — September 28, 2026

User requested a final build of both formats after confirming the VST3-only
installer did not install an AAX for Pro Tools. Fresh build directory:
`NewProject/build-windows-final-aax-vst3-2.0.0`.
Evidence: `Releases/Testing/2.0.0/windows-final-aax-vst3-20260928/`.
`build-manifest.json` and `source.patch` preserve source provenance. The source
includes the same local editor image-lifetime correction described above.

Both formats compiled successfully as x64 Release, version 2.0.0, with
`RCL_ENABLE_MOONBASE=ON`, `RCL_FINAL_RELEASE=ON`, standalone OFF, copying OFF,
and tests ON. Visual Studio 2022 used MSVC 19.43.34810 and Windows SDK
10.0.22621.0. Explicit JUCE and AAX paths point into the earlier final build's
pinned JUCE 8.0.15 source: `NewProject/build-windows-final-2.0.0/_deps/juce-src`
and its `modules/juce_audio_plugin_client/AAX/SDK` (AAX 2.9.0). Moonbase's
checksum-pinned 4.4.0 source was reused with `FETCHCONTENT_SOURCE_DIR_MOONBASE_CPP`;
the normal build-local hardening remains enabled.

The initial sandbox configure could not detect MSVC. Configuration and compilation
succeeded outside the sandbox with process Path/PATH normalization, `/MP1`,
`--parallel 1`, `/p:PreferredToolArchitecture=x64`, and
`/p:MultiProcessorCompilation=false`. All four targets were explicitly requested:
`RobinControlLite_VST3`, `RobinControlLite_AAX`, `RobinControlLiteTests`,
and `RobinControlLiteLicensingTests`.

- Engine/processor and licensing suites: PASS, exit 0.
- Unsigned and signed VST3: pluginval 1.0.4 strictness 10, GUI included,
  PASS with process exit 0. See `pluginval-*-report.txt` and `*-exit.txt`.
- Unsigned AAX: all 13 applicable checks PASS. DSP/HDX cycle count is N/A.
  See `aax-unsigned/summary.json`; the unchanged preference-preserving toolkit
  configuration has SHA-256 `73b38cbeb9d52ae97ac35e6473b526f79d6e5710952e0dd6935988ffb7c4c887`.
- Signed AAX validation: all 13 applicable checks PASS, runner exit 0;
  DSP/HDX cycle count N/A. See `aax-signed/summary.json`.
- PACE signature: PASS, Conduit DSP LLC / Robin Control Lite, **signed, not wrapped**.
  Timestamp: `2026-09-28T20:52:42Z`. Windows Authenticode: Valid, timestamped,
  CONDUIT DSP LLC. See `aax-pace-verify.log` and `aax-authenticode-verify.log`.
- Signed resource hashes match raw resources for both formats; see
  `signed-resource-comparison.json`.

Signed bundles are under the evidence directory's `signed/VST3/` and `signed/AAX/`.
Signed inner VST3 SHA-256:
`2B58613210F6056B5A065C73FD8081820435C312EB6548CD6FBEE13E31C7941A`.
Unsigned inner AAX SHA-256:
`5FDAFEE0F774159D31D73CA8E6CA46998DBA454C888064DE9C6DD180353A2F17`.
Signed inner AAX SHA-256:
`F39A82BC3C8CFC438FBE7FE1C2902B03A11DBA952F878FCF7D24B08B5630AADE`.

### Windows PACE signing details

`scripts/sign-windows-aax.ps1` combines the existing Lite signing-only PACE
configuration and physical developer iLok with Azure Authenticode signing.
WrapTool 6.0.1 supports `--signtool` and `--explicitsigningoptions` directly;
the explicit options begin with `sign /v /fd SHA256`, followed by Microsoft's
timestamp, dlib, and metadata options. Azure CLI and WrapTool use their existing
private caches; passwords/private keys are not placed in scripts or logs.

Two Windows-specific requirements were established from the tool's diagnostics:

1. Input/output bundle paths must be identical. The script first copies the
   unsigned bundle into a fresh staging path, then signs that copy in place.
2. `wraptool verify --in` requires the **inner file** at
   `Contents/x64/Robin Control Lite.aaxplugin`, not the bundle directory.

The first attempt with separate input/output paths did not sign anything.
The next attempt signed successfully but its directory-level verification failed;
verification of the inner file then passed without re-signing. Diagnostic logs
are retained as `sign-aax.log` and `sign-aax-staged.log`. The corrected script
uses both Windows requirements and refuses existing output paths.

`installer/windows/build-final.ps1` now optionally accepts `-AaxBundle` and
`-WrapTool`. It checks AAX identity, final licensing markers, notices/page table,
PACE and Authenticode signatures, publisher and timestamp before invoking Inno.
The installer adds the complete AAX bundle under the 64-bit Common Files Avid
plug-in directory and includes matching uninstall handling. Its beta-AAX guard
was exercised: it rejected `final=OFF` before packaging. It also rejected an
unsigned final AAX at the PACE verification gate.

The combined installer and its uninstaller were Azure-signed successfully.
Final installer verification passed with the expected publisher and timestamp.
Evidence: `installer-build.log`; package: `installer/RobinControlLite-2.0.0-Windows.exe`.
An identical copy is ready at
`Releases/Installers/RobinControlLite-2.0.0-Windows-AAX-VST3.exe`.
SHA-256: `FB72103665FA2D1E0BFA0ADE966FDEBA9B065EFFB8B85E901B5B0D4A17B552BA`.
`signed-payload-manifest.json` records all staged payload files, including AAX
bundle resources, and the build manifest records both AAX validation results.
The previous VST3-only installer is preserved under its original filename.

This combined installer has not yet been run on the machine. Installation/payload
comparison and retail Windows Pro Tools acceptance remain manual checkpoints;
earlier installation evidence above covers only the earlier VST3-only package.
Close audio hosts, run the new AAX-VST3 installer, then reopen Pro Tools and test
an Instrument track, activation, sample playback, and session save/reopen.
No upload, publication, commit, push, or customer licensing changes were made.

After packaging, the user requested removal of the installed VST3 for a fresh
install. The exact system bundle at
`C:/Program Files/Common Files/VST3/Robin Control Lite.vst3` was removed with
administrator elevation and its absence verified. Presets and license data
were preserved. This was bundle removal, not a full installer uninstallation.
The combined installer has not been confirmed installed or tested in Pro Tools.
