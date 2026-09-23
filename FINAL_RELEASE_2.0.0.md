# Robin Control Lite 2.0.0 — final macOS build

Updated 2026-09-22. Alex requested the final **2.0.0** build after a security and
cleanup pass. The binary/package version is 2.0.0, with no beta label. This is a
local final release build, **not a public Moonbase/website release**. Windows and
manual licensed-host release checks below remain separate.

This record supersedes earlier candidate status in `RELEASE_2.0.0.md` and the
DRM-free Beta 1 plan. `MOONBASE_BETA_1.md` remains historical private-beta evidence.
Read `WINDOWS_BUILD_AND_AAX.md` before moving to Windows and
`ROBIN_CONTROL_AGENT_HANDOFF.md` for premium-product reuse.

## Deliverables and identity

- Installer: `Releases/Installers/Robin Control Lite 2.0.0.pkg`
- Size: 25,153,216 bytes.
- SHA-256: `8173a65d9df1a5be537d7e3985874cd34f34f4da5e6d0fcdc24a259290b2d897` (also saved as `.pkg.sha256`).
- Signed staging: `Releases/Testing/2.0.0/final-20260922/signed/`
- Raw evidence: `Releases/Testing/2.0.0/final-20260922/`
- Formats: **VST3, AU, AAX**, each **arm64 + x86_64**, minimum **macOS 11.0**.
- Product/target: Robin Control Lite / `RobinControlLite`; manufacturer `Cdsp`,
  plugin code `rcll`, bundle ID `dsp.conduit.RobinControlLite`; parameter IDs kept.
- Standalone excluded; the build does not auto-install plugins.
- Final Moonbase flags: `RCL_ENABLE_MOONBASE=ON`, `RCL_FINAL_RELEASE=ON`.
- Online-license offline grace: **90 days**, subject to signed expiry/revocation.
  Separate permanent offline activation enabled. Dashboard limit: 10 activations;
  trials/auto-upgrade trials disabled. PACE is signing only, no customer iLok DRM.

The `.pkg` installs complete bundles to system VST3, Components and Avid plug-in
folders. It includes `MoonbaseNotices.txt`, `RCLBuildConfig.txt`, the AAX page table
and PACE symlinks. All payloads are non-relocatable; the installer EULA describes
v2.0.0 Moonbase activation. The user's existing beta installation is preserved;
AU validation temporarily registers a user-scope copy and restores that state.

## Reproduce the macOS build

Use a fresh directory from the repository root. This Mac uses Command Line Tools
and macOS 15.4 SDK to avoid its previously observed Xcode 27 SDK mismatch:

```sh
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  -S NewProject -B NewProject/build-final-2.0.0 -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64' \
  -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk \
  -DCMAKE_C_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DCMAKE_OBJC_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_OBJCXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON \
  -DRCL_BUILD_STANDALONE=OFF -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  --build NewProject/build-final-2.0.0 \
  --target RobinControlLite_VST3 RobinControlLite_AU RobinControlLite_AAX -j 4
```

Local JUCE: `~/Documents/JUCE`, commit
`91ad83ae34a81e0833b1a2b0866f54846370ae53` (8.0.15). FetchContent uses the same
commit archive with SHA-256
`04f8d5055382582c757be9da069ea98338005f98248facd9c2804435ac853e70`.
AAX SDK: `~/SDKs/aax-sdk-2-9-0`; pass explicit paths on another machine.
Moonbase 4.4.0 archive SHA-256:
`c3f2ea70dabb40a71cc0502fa07d82e18e2e6fdbd548165d2ec78272c327dede`.
This build reused its previously downloaded source through
`-DFETCHCONTENT_SOURCE_DIR_MOONBASE_CPP=/tmp/moonbase-cpp-4.4.0`; omit that override
on a fresh clone so CMake performs its checksum-verified download.

`NewProject/cmake/MoonbaseHardening.cmake` applies reviewed corrections to a
build-local module copy. It leaves the source SDK unchanged, fails on unexpected
patch context, and passed repeated configure testing. Build profile records
`moonbase_sdk=4.4.0-rcl-hardening-1`. See `SECURITY_REVIEW_2.0.0.md` for scope.

Stage fresh copies; never rebuild or postprocess signed staging. Follow
`AAX_BUILD_AND_SIGNING.md` for Lite's PACE `sign` workflow. Use Developer ID
Application `CONDUIT DSP LLC (QS378YGT2W)` for VST3/AU and through PACE for AAX.
Verify Apple signatures outside the restricted sandbox: it produced false
invalid-signature diagnostics, while unrestricted verification passed all three.
PACE also emitted an existing-input-signature diagnostic during signing; both
independent verifiers passed on the output. PACE confirms **signed, not wrapped**.

Package only those exact signed copies:

```sh
RCL_ARTEFACTS="$PWD/Releases/Testing/2.0.0/final-20260922/signed" \
RCL_INCLUDE_AAX=1 \
INSTALLER_SIGN='Developer ID Installer: CONDUIT DSP LLC (QS378YGT2W)' \
  bash installer/build-installer.sh
```

Do not set `RCL_BETA_LABEL`. Final packaging refuses missing DRM/final build
markers, mismatched versions, missing notices or invalid signatures. It refuses
to overwrite an existing installer. Notarize using the existing `AC_PASSWORD`
Keychain profile, then staple and validate; never retrieve a password into chat
or logs. Local paths/evidence are ignored by Git and do not transfer to Windows.

## Verification

| Check | Result |
| --- | --- |
| Universal Release VST3/AU/AAX compilation | PASS, version 2.0.0, both architectures, minimum OS 11.0 |
| Engine/processor regressions | PASS |
| Engine + licensing ASan/UBSan | PASS; leak detection disabled |
| Engine + licensing TSan | PASS |
| Signed VST3 pluginval strictness 10, GUI included | PASS on native arm64 and Rosetta x86_64 |
| Final AU native and Rosetta auval / pluginval strictness 10 | PASS; mapped-binary paths confirm final AU was tested |
| AAX before PACE signing | PASS: all 13 applicable checks; HDX cycle-count N/A |
| AAX after PACE signing | PASS: all 13 applicable checks; HDX cycle-count N/A |
| Apple strict signatures / PACE AAX verifier | PASS |
| Extracted installer vs signed staging | PASS: file hashes, modes and symlink targets match all formats |
| Installer identity and updated EULA | PASS |
| Apple notarization / staple / Gatekeeper | Accepted / valid / accepted |

Notarization submission: `bc2927ec-30be-44ce-ad57-57ec231009c3` (**Accepted**).
Toolchain: JUCE 8.0.15, AAX SDK 2.9.0, PACE 6.0.1, DigiShell 24.9.0x14.
Meaningful regressions cover malformed multichannel/NaN samples, tiny resampling,
oversized/null host state and oversized/Unicode license caches. Synthetic license
fixtures test signature/product/device/expiry/grace and asynchronous activation
lifecycle without issuing real customer seats. A limited credential-pattern scan
found only intended synthetic JWT fixtures, not production secrets.

The source was built from HEAD `68bc4ba` plus the local audited changes described
in this record. The evidence manifest records hashes of build/source inputs;
commit and push this full change set before cloning for the Windows build.
Do not identify a remote HEAD alone as the source of these binaries.

## Remaining public-launch checkpoints

- [ ] Test the **exact final** licensed binaries in real DAWs: activated playback,
  save/reopen/editor closed, multiple instances, offline bounce and host unload.
  Previous user-confirmed AAX acceptance and beta delivery are narrower evidence.
- [ ] Real permanent offline request/response exchange, disconnected restart,
  deactivation/revocation and network failures. Do not change the production
  machine's clock or delete a real license to test grace boundaries.
- [ ] Windows x64 final build/tests, VST3 validation, Windows AAX signing/validation
  and retail Pro Tools acceptance, then Windows packaging. Mac results do not
  satisfy these checks; Windows compiler/runtime is not available here.
- [ ] Reconcile the website privacy/EULA/download flow with the updated repository
  disclosures. The website was not edited or published by this task.
- [ ] Set up public free-license acquisition/existing-user migration. Current
  product is not purchasable/autoprovisioned; it is configured for private tests.
- [ ] Upload the final installer only when ready, then update Moonbase fulfillment
  text and links. Current email text points at private beta release **1.0.1**.
  The accidental ACTIVE **1.0.0** dashboard entry remains a support cleanup task.
  Neither label is the final binary version, and this task published nothing.

Automated results and notarization support distribution integrity; they do not
replace the manual playback/activation checks above or certify vulnerability-free
software. Preserve evidence and signed artifacts when moving to another machine.
