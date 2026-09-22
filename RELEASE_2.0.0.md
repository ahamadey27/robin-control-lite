# Robin Control Lite 2.0.0 release checkpoints

Started 2026-09-20. **Release candidate; not published.** This file is the current
2.0.0 execution record; `release-spec.md` retains historical v1.0 evidence.
See `AAX_BUILD_AND_SIGNING.md` for the reusable AAX handoff and
`WINDOWS_BUILD_AND_AAX.md` for a fresh Windows clone/build.

## Scope

User requested v2.0.0, built as VST3, AU, and AAX. This pass builds universal
macOS binaries (Intel x86_64 + Apple Silicon arm64). Windows release builds and
host tests remain separate work. Standalone stays available for local F5 testing
and is excluded from this release build. Plugin identity and parameter IDs stay
compatible with existing Lite sessions.

Alex explicitly confirmed **no customer iLok DRM** on 2026-09-20. AAX uses
developer code signing only, with no Lite iLok license activation, customer iLok
account, USB key, or Cloud requirement. Pro Tools' own licensing is separate.

Alex subsequently specified **Moonbase DRM for free Lite and all future
products**. This is the chosen customer licensing provider; PACE remains limited
to AAX signing. Alex clarified that Moonbase is required for **final v2.0.0**
but excluded from **Beta 1** (called “beta v1” in conversation). Do not block
Beta 1 on Moonbase or infer a binary version change from that label. Beta 1
still requires AAX signing for retail Pro Tools. No Moonbase integration was
performed in this release preparation work, and the existing build/test evidence
does not cover it. Activation policies remain unspecified.

## Checkpoints

- [x] Implement opt-in Moonbase JUCE licensing, processor audio gate, and activation UI.
- [ ] Final release only: complete live Moonbase activation/host/platform validation;
      build with `RCL_ENABLE_MOONBASE=ON` and `RCL_FINAL_RELEASE=ON`.
      Beta 1 remains DRM-free. See `MOONBASE_INTEGRATION.md` for separate macOS
      and Windows commands and new evidence; earlier candidate evidence below
      does not validate the licensing-enabled binary.
- [x] Version set to 2.0.0 in authoritative CMake and Projucer metadata.
- [x] Plugin/test version macros derive from the CMake project version.
- [x] AAX SDK, validator, and PACE tool installation located.
- [x] User-supplied PACE welcome email confirms signing-only SDK access;
      application specifies no Cloud AAX Signing. No customer PACE DRM authorized.
- [x] Apple Developer ID Application identity checked outside sandbox.
- [x] Build universal Release VST3, AU, and AAX.
- [x] Verify all three bundle versions: 2.0.0; architectures: x86_64 + arm64;
      Mach-O minimum OS: 11.0 on both slices; SDK: 15.4.
- [x] Run unit tests against the 2.0.0 sources — passed.
- [x] Release VST3 pluginval strictness 10 — passed, reports version 2.0.0.
- [ ] Release AU validation confirmed as 2.0.0 — see registry caveat below.
- [x] Pre-sign AAX Native validation — all 13 applicable checks passed September 22;
      Avid's DSP/HDX cycle-count test is N/A, with evidence below.
- [x] ASan/UBSan and TSan release checks — both clean.
- [x] Installer rejects stale 1.0.1 artifacts when source version is 2.0.0.
- [x] Confirm signing iLok certificate-seal indicator in post-sync screenshot.
- [x] Obtain active product and SDK 6 Signing Only Wrap GUID from PACE Central.
- [x] Confirm PACE account/publisher configuration through successful signing and verification.
- [x] Sign a separate AAX candidate; PACE and strict Apple signature verification pass.
- [ ] Sign final staged VST3/AU and AAX; verify both Apple and PACE signatures.
- [x] Validate corrected, signed AAX — all 13 applicable checks passed September 22.
- [x] Install corrected signed AAX for Pro Tools testing; installed PACE/Apple
      signatures and every file hash/symlink match the validated candidate.
- [x] Basic retail Pro Tools acceptance — Alex reports the installed corrected
      AAX working in regular Pro Tools / Intro on September 22.
- [ ] Complete the detailed Pro Tools release matrix; the user's basic success
      report does not establish every individual test below.
- [ ] Extend installer and uninstaller for the signed AAX.
- [ ] Sign, notarize, staple, and inspect the final three-format installer.
- [ ] Complete clean-install and real-host smoke checks from `TESTING.md`.
- [ ] Windows builds, signing decisions, validation, and packaging.
- [ ] Publish/tag only when the intended release artifacts and host evidence are ready.

## Reproducible macOS candidate build

The historical Xcode-generator directory `NewProject/build-release` failed to
configure with Xcode 27: deployment target 11.0 is below its supported range,
and its device frameworks also reported version mismatches. Do not raise Lite's
minimum OS to work around this. The candidate uses a separate Makefiles build,
the installed Command Line Tools compiler, and the macOS 15.4 SDK.

Local JUCE: `/Users/alex/Documents/JUCE`, clean Git checkout at
`91ad83ae34a81e0833b1a2b0866f54846370ae53` (8.0.15). This is newer than the
8.0.4 FetchContent fallback used on fresh machines/CI; record this distinction
when comparing results.

From repository root:

```sh
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  -S NewProject -B NewProject/build-release-2.0.0 -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release \
  '-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64' \
  -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk \
  -DCMAKE_C_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DRCL_BUILD_STANDALONE=OFF -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF \
  -DRCL_BUILD_TESTS=OFF

DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  --build NewProject/build-release-2.0.0 --config Release \
  --target RobinControlLite_VST3 RobinControlLite_AU RobinControlLite_AAX -j 6
```

The explicit developer directory also keeps JUCE's helper-tool build from mixing
the CLT compiler with the incompatible Xcode 27 SDK. Automatic plugin copying
is disabled so these candidates do not replace the working installed plugins.

Artifact root: `NewProject/build-release-2.0.0/RobinControlLite_artefacts/Release/`

- `VST3/Robin Control Lite.vst3`
- `AU/Robin Control Lite.component`
- `AAX/Robin Control Lite.aaxplugin`

**Installer path warning:** the installer defaults to `NewProject/build-release`,
which contains older artifacts. It now rejects bundle versions that do not match
the project version. Set `RCL_ARTEFACTS` to the final signed staging root when
packaging; do not point it at the unsigned candidates. AAX inclusion is still a
separate pending installer change.

## Evidence and next checkpoint

Evidence is retained in `Releases/Testing/2.0.0/` (local, ignored by Git),
including `artifact-manifest.json` with executable SHA-256 hashes and exact
build metadata. No 2.0.0 release installer has been built or published.
The corrected AAX has verified PACE/Apple signatures as recorded below;
final staged VST3/AU signing and release notarization remain open.

**AU registry caveat:** pluginval and auval returned success, including native
and Rosetta auval runs, but macOS continued to report **1.0.1** in its component
registry. This happened even when the new AU was temporarily copied into both
the user and system Lite AU bundle locations and the registrar was restarted.
These passes are not accepted as conclusive 2.0.0 AU validation. Resolve the
registry/version discrepancy, then rerun. Both original installed AU bundles
were restored after testing. Do not report the temporary test install as a
completed 2.0.0 installation.

**AAX validator investigation — September 22:** DigiShell v24.9.0x14's original
September 20 `runtests` run exited 0 despite two failures. The page-table failure
was caused by the missing registered XML resource, now added through JUCE's
`AAXClientExtensions` and packaged by CMake. The rebuilt universal 2.0.0 AAX
passes loading and automation-list validation with all 26 active parameter IDs
plus JUCE's master bypass.

`test.cycle_counts` is an Avid **AAX DSP/HDX** test, inapplicable to Lite's
Native-only descriptor. Its helper also terminates on `load_dish DAE` with no
plugin loaded (exit 255); the original trace records Mach exception 1 / signal
11 at that step. The toolkit was not patched and that independent DAE failure
was not repaired. Record this check as **N/A, never PASS**. The new
`scripts/test-aax-native.py` confirms Native-only types before excluding just
this test, runs all other installed tests, and rejects failed/incomplete results.
Evidence: `Releases/Testing/2.0.0/aax-native-20260922/`.

**Result:** all 13 applicable validator checks passed on the rebuilt universal
unsigned 2.0.0 AAX, including both page-table checks, all three parameter traversal
modes, parameter behavior, lifecycle, and 1,000 load/unload iterations. The rebuilt
unit/processor suite also passed. The result parser was checked against actual
PASS output and synthetic lost, failed, canceled, aborted, missing, duplicate,
mismatched, and helper-error results; it rejected every failure case. Windows
resource packaging is wired but still requires a Windows build/validation run.

The rebuilt AAX replaces the unsigned build output; the original
`artifact-manifest.json` remains historical September 20 evidence. Use the new
run's `summary.json` for the rebuilt executable/resource hashes. The September
20 signed bundle remains unchanged and does **not** contain the page-table fix.
Moonbase integration remains deferred and will require a fresh validation pass.

**Corrected signing checkpoint — September 22:** the connected developer iLok
and WrapTool's default cached credentials signed the corrected AAX into
`Releases/Testing/2.0.0/signing-validated-20260922/AAX/Robin Control Lite.aaxplugin`.
Independent PACE and strict Apple verification passed outside the sandbox.
PACE reports **signed, not wrapped**, timestamp `2026-09-22T13:30:21Z`;
Apple reports the Conduit DSP Developer ID and hardened runtime. Both x86_64
and arm64 still target macOS 11.0. The page-table XML is unchanged and included
in the signed bundle; the unsigned input is unchanged.

Signing again printed a diagnostic about validating the input's existing
signature but exited 0; independent output verification passed. Evidence is in
`signing-validated-20260922/signing-evidence.json` and adjacent verification logs.
All 13 applicable signed-bundle checks passed; results are under
`aax-native-signed-20260922/`. The candidate was installed using `ditto` to
`/Library/Application Support/Avid/Audio/Plug-Ins/Robin Control Lite.aaxplugin`
while Pro Tools was closed. No prior Lite AAX existed at that location.
Every installed file hash and symlink matches the verified candidate, and both
installed PACE and strict Apple signature verification passed. Installation
evidence is `signing-validated-20260922/installation.json` and adjacent logs.

Alex confirmed the test host is regular Pro Tools / Intro; installed application
metadata reports version `26.4.1.179`. On September 22 Alex reported: **“this is
working in pro tools.”** Record basic retail-host acceptance for the exact
installed signed candidate. No host logs or itemized test results were supplied.
Do not infer that both mono/stereo tracks, Trigger/Panic, all UI sizes, automation,
preset/session restore, or absence of Lite activation prompts were separately
verified. Those remain in the detailed release matrix.
Customer testing without developer signing entitlements remains part of the host
matrix; Pro Tools' own licensing is separate. Notarization remains pending.
This candidate does not include Moonbase.

PACE 6.0.1 is installed. The welcome email confirms digital-signing-only SDK
access, and the application receipt specifies no Cloud AAX Signing. Subsequent
screenshots and portal details establish the developer licenses, certificate
indicator, and product configuration. End-to-end AAX signing now succeeds.

**Historical first signing checkpoint — September 20:** Alex supplied the active product and Signing Only
configuration with Wrap GUID `8F95C7F0-B538-11F1-8437-00505692AD3E`.
The first trial stopped with a missing-password error. After Alex completed local
authentication, the retry succeeded. PACE and strict Apple verification both
passed outside the sandbox. PACE confirms **signed, not wrapped**; Apple confirms
the Conduit DSP Developer ID, timestamp, and hardened runtime. The signed copy
retains version 2.0.0 and both architectures; the original input is unchanged.

Signed AAX: `Releases/Testing/2.0.0/signing-trial/AAX/Robin Control Lite.aaxplugin`.
Evidence: `Releases/Testing/2.0.0/aax-signing-evidence.json`.
This is a historical test candidate; use the corrected September 22 signing
output above for further testing. Retail Pro Tools tests, remaining format
signing, packaging, and notarization remain open.
See the AAX handoff for signing diagnostics and symlink-preserving copy guidance.

Further research found a working PACE documentation sign-in route behind the
email URL; automated web-tool failure was not evidence of a missing document.
The user has now synchronized the physical iLok and supplied a close-up clearly
showing the certificate seal. Developer tool license names and their displayed
2027-10-01 expiration are also confirmed from the screenshots. The product and
signing-only configuration have now been created; no need to wait for a GUID
by email or repeat those setup steps. See the AAX handoff for sources and
the completed authentication/signing setup and remaining release work.
