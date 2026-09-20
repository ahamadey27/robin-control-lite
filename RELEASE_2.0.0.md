# Robin Control Lite 2.0.0 release checkpoints

Started 2026-09-20. **Release candidate; not published.** This file is the current
2.0.0 execution record; `release-spec.md` retains historical v1.0 evidence.
See `AAX_BUILD_AND_SIGNING.md` for the reusable AAX handoff.

## Scope

User requested v2.0.0, built as VST3, AU, and AAX. This pass builds universal
macOS binaries (Intel x86_64 + Apple Silicon arm64). Windows release builds and
host tests remain separate work. Standalone stays available for local F5 testing
and is excluded from this release build. Plugin identity and parameter IDs stay
compatible with existing Lite sessions.

Alex explicitly confirmed **no customer iLok DRM** on 2026-09-20. AAX uses
developer code signing only, with no Lite license activation, customer iLok
account, USB key, or Cloud requirement. Pro Tools' own licensing is separate.

## Checkpoints

- [x] Version set to 2.0.0 in authoritative CMake and Projucer metadata.
- [x] Plugin/test version macros derive from the CMake project version.
- [x] AAX SDK, validator, and PACE tool installation located.
- [x] User-supplied PACE welcome email confirms signing-only SDK access;
      application specifies no Cloud AAX Signing. No customer DRM authorized.
- [x] Apple Developer ID Application identity checked outside sandbox.
- [x] Build universal Release VST3, AU, and AAX.
- [x] Verify all three bundle versions: 2.0.0; architectures: x86_64 + arm64;
      Mach-O minimum OS: 11.0 on both slices; SDK: 15.4.
- [x] Run unit tests against the 2.0.0 sources — passed.
- [x] Release VST3 pluginval strictness 10 — passed, reports version 2.0.0.
- [ ] Release AU validation confirmed as 2.0.0 — see registry caveat below.
- [ ] Pre-sign AAX full validation — unresolved failures below.
- [x] ASan/UBSan and TSan release checks — both clean.
- [x] Installer rejects stale 1.0.1 artifacts when source version is 2.0.0.
- [ ] Confirm Avid/PACE account setup, publisher/configuration, and signing iLok.
- [ ] Sign final staged VST3/AU and AAX; verify both Apple and PACE signatures.
- [ ] Validate signed AAX and complete retail Pro Tools host smoke test.
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
build metadata. No 2.0.0 release installer has been built or published. The
candidate bundles have not been distribution-signed or notarized.

**AU registry caveat:** pluginval and auval returned success, including native
and Rosetta auval runs, but macOS continued to report **1.0.1** in its component
registry. This happened even when the new AU was temporarily copied into both
the user and system Lite AU bundle locations and the registrar was restarted.
These passes are not accepted as conclusive 2.0.0 AU validation. Resolve the
registry/version discrepancy, then rerun. Both original installed AU bundles
were restored after testing. Do not report the temporary test install as a
completed 2.0.0 installation.

**AAX validator:** the SDK-path executable reports DigiShell v24.9.0x14. The
full stdin `runtests` run returned exit code 0 but was **not an overall pass**:
`test.page_table.load` failed for two realtime types, reporting “Failed to load
page tables library”; `test.cycle_counts` returned `E_LOST` with a broken-pipe
helper error. The other reported stages passed. Investigate these two checks
before claiming AAX release readiness. The last result in the log is PASS but
does not summarize the whole run.

AAX signing setup is still unverified. PACE 6.0.1 is installed, and Alex reports
“Pace Tools”, “Pace Central Access”, and “Edan Tools” entitlements on the physical
iLok. The supplied welcome email confirms digital-signing-only SDK access;
the application receipt specifies no Cloud AAX Signing. Neither supplies a
publisher/configuration identifier or confirms a working signing certificate.
Next, open PACE Central through iLok License Manager and consult the authenticated
signing setup documentation, as described in `AAX_BUILD_AND_SIGNING.md`.
Confirm the active distribution entitlement before publishing. No PACE signing
has been attempted.
