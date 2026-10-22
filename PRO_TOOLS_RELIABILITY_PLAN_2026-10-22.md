# RCL and RC — Pro Tools reliability execution plan

Started October 22, 2026. Owner: Alex / conduit.dsp. Update this file after each
verified checkpoint; an unchecked item is pending, not an implied failure.

## Objective and current checkpoint

Resolve the Lite 2.0.0 interface crash reported inside Pro Tools / Blue Cat's
PatchWork, investigate the separately reported startup crash, and prevent the
same build and lifecycle risks in Robin Control premium.

**Current checkpoint: automated macOS validation passes for both products'
signed diagnostic candidates. Real-host/platform/release gates remain open.** Pool
mutation APIs and XML preflight are implemented; native Debug engine/licensing
tests, ASan/UBSan and TSan pass. The combined RCL candidate matches committed
source `65dc245`; all previously recorded source and binary hashes still match.
Apple Rosetta 2 was installed on macOS 27.0.1, resolving the earlier `Bad CPU
type` failures. Both products' new signed VST3 copies pass strictness 10 with GUI
tests on ARM64 and Intel. RCL unsigned and PACE-signed AAX each pass all 13
applicable checks. Both products' signed AUs pass native and Intel auval plus
strictness-10 GUI pluginval with exact candidate identification. All temporarily
changed AU installations are restored and verified. RC unsigned and signed AAX
each pass all 13 applicable checks, plus Apple/PACE/export verification.
No corrected candidate has yet been accepted by the affected customer. No
hotfix is published.
The original signed 2.0.0 artifacts and installed plug-ins must remain intact.

The authoritative tracker is this file in `robin-control-redesign`. A matching
handoff copy is kept in `round-robin-premium`; reconcile it from this file after
updating progress. The earlier [crash audit](../round-robin-premium/PRO_TOOLS_CRASH_AUDIT_2026-10-22.md)
records the original evidence and the six findings F1–F6.

## Established facts and preserved boundaries

- Lite baseline: `33b4398`, version 2.0.0, JUCE 8.0.15 at
  `91ad83ae34a81e0833b1a2b0866f54846370ae53`.
- Customer report: `crash-reports/2026_10_22_13_14_58_cd22bbd0-09ab-4802-a7ba-3c9471dc4842.crash`.
  Pro Tools 24.6.0 build 149, native Apple Silicon, macOS 14.2; PatchWork version
  still needed. This dump captures the VST3 editor opening inside PatchWork, not
  a separate boot-screen crash.
- UUID-matched VST3 symbolication proves an invalid free of static
  `juce::emptyString` while constructing a slider. Exposed internal JUCE symbols
  strongly support cross-module symbol collision as the leading diagnosis.
- Old final signed ARM64 export counts containing `juce`: AAX 8,726, VST3 8,741,
  AU 8,726. C++ objects hide symbols; Objective-C++ objects lack that flag.
- Prepared RC macOS 1.0.2 signed artifacts have zero JUCE exports in both slices;
  RC already contains XML preflight and editor-owned software artwork.
- Keep product/manufacturer codes, bundle IDs, APVTS/parameter IDs, MIDI behavior,
  additive loading, Moonbase policy and PACE signing-only behavior unchanged.
- Final-profile Lite candidates require Moonbase ON and FINAL_RELEASE ON;
  copy/install OFF. No production activation data is a test fixture.
- Use fresh build/evidence/staging folders. Never rebuild into old signed
  staging. No customer messages, uploads, commerce changes or public releases
  are authorized solely by this engineering plan.
- Diagnostic candidate may retain 2.0.0 while isolating F1. General-release
  target is RCL 2.0.1 after the fixes and release checks below. Record candidate
  identity using source/build manifests rather than relying on its version alone.
- The historical macOS 15.4 SDK is no longer installed. Record the actual SDK
  used for this work and verify the minimum OS/architecture metadata separately.

## Phase 0 — Preserve and identify the baseline

- [x] Read the supplied crash and match its VST3 UUID to final 2.0.0 staging.
- [x] Preserve original report and signed bundles; retain original package hashes.
- [x] Record the invalid-free address and stack in the crash audit.
- [x] Compare RC's prepared macOS binary exports.
- [x] Record current source, dependency versions, toolchain, commands and build
  flags in a new candidate manifest.
- [ ] Obtain PatchWork version and separate boot-screen dump when available.

Evidence from the investigation lives in ignored
`Releases/Testing/2.0.0/pro-tools-audit-20261022/`. Do not commit the minidump or
raw trace; they contain private diagnostic material.

## Phase 1 — Isolate F1: macOS visibility and export regression gate

### 1.1 Implement the narrow build correction

- [x] Explicitly hide C/C++/Objective-C/Objective-C++ symbols for shared code and
  every generated format target, including builds with/without Moonbase.
- [x] Add a product-independent macOS export checker. It must fail on leaked
  internal JUCE/Moonbase C++ symbols or missing host entry points, inspect every
  architecture, and emit reviewable JSON.
- [x] Wire that checker into the Mac CI build without claiming a hosted run.
- [x] Keep this first candidate free of sample/DSP/licensing behavior changes.

### 1.2 Build in isolation

Use `NewProject/build-pt-visibility-20261022` for the focused production-profile
candidate, auto-copy OFF. Build VST3/AU/AAX explicitly so a missing AAX SDK cannot
silently omit AAX. Keep the JUCE/Moonbase versions pinned and record how a cached
dependency was verified or downloaded. Save configure/build logs and compile
commands. Do not reuse old final output folders.

- [x] Configure universal Release, Moonbase ON, FINAL_RELEASE ON.
- [x] Inspect generated `.mm` compiler commands for `-fvisibility=hidden`.
- [x] Build all three plug-in formats.
- [x] Run export gate on each format and both architecture slices.
- [x] Confirm intended VST3/AU/AAX entry points and bundle resources survive.
- [x] Record candidate executable hashes, UUIDs, minimum OS and architecture.
- [x] Preserve this candidate before implementing F2/F3.

Evidence: `visibility-source-manifest.json`, `visibility-source.diff`,
`visibility-configure.log`, `visibility-build.log`, `visibility-compile-flags.json`,
`visibility-exports.json` and `visibility-artifact-manifest.json` in the audit
evidence folder. All 35 `.mm` compile commands include hidden visibility.
All six format/architecture combinations have zero internal exports and retain
their required entry points. All slices record minimum macOS 11.0 / SDK 26.5.
The pinned Moonbase archive checksum and all 148 cached source files were verified
before reuse from RC's dependency cache. Old Lite AAX fails the new export gate;
prepared RC AAX/VST3/AU pass it in both slices.

**Acceptance:** no unintended internal JUCE/Moonbase exports; format entry points
present; all formats build; output is distinct from the original signed release.
This verifies the build defect, not the customer's outcome.

### 1.3 Validate and reproduce the original trigger

- [x] VST3 pluginval strictness 10 with GUI tests, native ARM64.
- [x] VST3 pluginval strictness 10 with GUI tests, Rosetta Intel.
- [x] AU candidate validation with conclusive candidate path/version evidence;
  preserve/restore any installed AU temporarily needed by the registry.
- [x] Applicable AAX Native checks on the new unsigned candidate.
- [x] Stage fresh RCL signed binaries; independently verify Apple/PACE and
  exports after signing.
- [x] Repeat applicable AAX checks against the exact signed RCL candidate.
- [ ] Retail Pro Tools: scan/launch, instantiate native AAX, open/close editor
  repeatedly on mono/stereo instrument tracks.
- [ ] PatchWork inside Pro Tools: load candidate VST3, open/close editor, unload
  and reload; record PatchWork and host versions and exact loaded binary UUID.
- [ ] Repeat with normal plug-in collection and a reduced collection if useful
  for load-order diagnosis. Restore each test setup.
- [ ] Affected customer retests the exact identified candidate; record result.

Initial F1-only evidence: `visibility-pluginval-native.log` and its output directory;
Editor, Open editor whilst processing and Editor Automation all execute and pass.
The initial Rosetta launch failed before loading the candidate; `/usr/bin/true`
also could not execute as Intel. The Avid helper's bundled Intel Ruby could not
start either. These initial runs predated Rosetta installation. Keep their
aborts visible; the successful complete combined-candidate reruns are separate
evidence below.

The initial F1-only unsigned AAX run records **10 PASS / 3 aborted FAIL**:
`info.productids`, `info.support.general` and `test.data_model` cannot start the
bundled Intel Ruby. Loading/unloading, description, page tables, all three
parameter traversals, parameter behavior and the other native checks pass.
`visibility-aax-unsigned/summary.json` correctly retains `passed: false`; do not
turn the environmental aborts into exemptions or change them to PASS.

If exports are clean but the same invalid free persists, preserve the new dump
and investigate actual cross-module `var`/String function selection. Do not mark
the issue fixed solely because a validator passed. No separate startup claim is
closed until its report/reproduction has been evaluated.

### October 22 continuation — restored Intel execution and signed staging

The initial environmental aborts above are preserved as historical results.
Apple Rosetta 2 installation succeeded; Intel `/usr/bin/true` now exits 0.
`validation-environment-continued.json` records macOS 27.0.1 (26A434), the
Rosetta receipt and committed Lite source `65dc245`. All candidate source/binary
hashes were checked before further validation; neither build was regenerated.
Apple documents Rosetta installation and availability through macOS 27:
[Using Intel-based apps on a Mac with Apple silicon](https://support.apple.com/en-us/102527).

`reliability-pluginval-rosetta.log` and
`rc-visibility-pluginval-rosetta.log` record successful Intel strictness-10 GUI
runs on the unsigned candidates. Fresh Developer ID staging is under
`reliability-signed/` and `rc-visibility-signed/` in the audit evidence folder.
VST3/AU in both products pass strict Apple verification, expected identity /
timestamp / hardened-runtime checks, both-slice export checks and resource
preservation. `signed-vst3-validation-summary.json` records **4/4 PASS**, native
and Intel strictness-10 GUI validation of these exact signed VST3 copies.

`reliability-aax-unsigned/summary.json` records **13/13 PASS** for the combined
RCL candidate; only the Native-inapplicable HDX cycle test is N/A. The new RCL
signed AAX independently passes PACE and Apple verification and both-slice
export checks. PACE identifies Robin Control Lite and **signed, not wrapped**.
Its signing log includes the previously observed input-signature diagnostic;
acceptance is based on both independent verifiers of the output, not that
diagnostic. All unsigned build outputs remain unchanged.
`reliability-aax-signed/summary.json` separately records **13/13 PASS** for the
exact signed output, executable SHA-256
`e8530e7784491f78e84b5af347f41b26421564fc90b2e0b1cb334ba8d4f351c1`.
Both summaries' hashes were rechecked against their respective executable.

The first AU attempt (`combined-au-validation/`) correctly records overall
failure of candidate identification: native auval selected the original system
AU, and native pluginval mapped both system and candidate copies. These are
**inconclusive candidate results**, not proved AU compatibility failures.
Intel auval/pluginval mapped only the candidate and passed for each product.
Both prior user copies were restored with matching file hashes/modes/symlinks;
system copies were unchanged. Do not erase or relabel the first attempt.

With temporary system-copy isolation, RC passes all **4/4** exact-candidate AU
checks in `combined-au-isolated-validation/summary.json`; prior user/system
copies were restored and verified in `au-system-isolation-summary.json`.
That attempt retains an overall failure because Lite's registry still selected
the removed system path and could not instantiate it. A subsequent privileged
copy attempt was blocked from reading Documents; its rollback also restored the
original system AU, and no validation result was claimed for that attempt.

The final Lite run temporarily placed identical, inventory-verified signed
copies at both registered user/system paths, using verified temporary staging
for privileged file access. `rcl-au-matched-copies-validation/summary.json`
records **4/4 PASS**: auval and strictness-10 GUI pluginval on ARM64 and Intel.
Every mapped path resolves to the verified candidate bytes, with no original
release mixed into the process. `rcl-au-system-isolation-v2-summary.json`
records overall success and restoration of the original system bundle; the
validation summary verifies prior user restoration. File hashes, modes and
symlink targets match the original inventories. No AU cache/preferences were
deleted, and no permanent plug-in replacement was retained.

`signed-candidate-metadata.json` rechecks all 12 signed format/architecture
combinations: expected universal slices, Lite minimum macOS 11.0, RC minimum
13.0, SDK 26.5 and unchanged signed executable hashes. The consolidated
`validation-checkpoint-summary.json` ties the four complete AAX runs, four
signed VST3 GUI runs and eight exact AU checks to their evidence. It explicitly
records real-host/customer/Windows checks as not run and publication as not
performed. Failed/environmental attempts remain preserved separately.

| Signed candidate | ARM64 auval | ARM64 GUI pluginval 10 | Intel auval | Intel GUI pluginval 10 | Prior installations restored |
| --- | --- | --- | --- | --- | --- |
| RCL 2.0.0 combined diagnostic | PASS | PASS | PASS | PASS | Verified |
| RC 1.0.2 prevention diagnostic | PASS | PASS | PASS | PASS | Verified |

**Next real-host checkpoint for Lite:** the automated Lite gates above pass; use
`reliability-signed/AAX/Robin Control Lite.aaxplugin` for a reversible retail
Pro Tools test. The local retail host is **26.4.1.179**, different from the
customer's **24.6.0 build 149**. Close Pro Tools first, inventory/back up the
installed whole AAX outside the scanned directory, install only this signed
bundle, then verify every file/mode/symlink, Apple/PACE signatures and loaded
ARM64 UUID `3BBFF262-DB0B-3E37-A892-0079AD3A45B9`. Use a new disposable session
for native AAX cold launch, mono/stereo insertion and repeated editor open/
close, then the Phase 5 playback matrix. Restore and verify the prior installed
bundle after the diagnostic run. No PatchWork installation was found in the
local AAX folder, so native Pro Tools success alone cannot close the original
PatchWork VST3 trigger or establish affected-customer acceptance.

## Phase 2 — RCL F2: make pool mutations atomic with respect to playback

- [x] Add processor-owned APIs for Add, Replace, Clear and Delete/compaction.
- [x] Keep decoding/resampling outside the audio callback lock; commit prepared
  slot changes plus loaded/shuffled lists and playback indices under that lock.
- [x] Move UI mutation code to those APIs; eliminate direct unlocked slot edits.
- [x] Enforce locking for public rebuild/shuffle helpers or make their contract
  explicit and private. Review state restore and sample-rate lifecycle too.
- [x] Preserve additive loading, series/random semantics, valid sample order and
  active-voice storage lifetime across all mutations.
- [x] Extend concurrency tests through the actual new APIs for Add/Replace/
  Clear/Delete while audio/MIDI, automation, save/restore and audition run.
- [x] Test sparse/full pools, invalid indices/files, failure rollback, rate
  changes during decode, repeated clear, delete first/middle/last and refill.
- [x] Engine tests, ASan/UBSan and TSan pass with these operations included.

**Acceptance:** every shared mutation is serialized; invalid operations are safe;
no new race, use-after-free or unintended replace-all behavior.

Implementation: `addPoolSamples`, `replacePoolSample`, `clearSamplePool` and
`deletePoolSample` own the callback-lock commit, including both playback lists
and sound refresh. `SampleLoader` separates preparation from commit and rejects
stale-rate prepared data. Failed replacement preserves the existing slot. The
concurrency harness now invokes these actual APIs; native engine tests pass.
This does not replace manual listening or establish every possible UI/state
interleaving; those remain in the host matrix and sanitizer checks.

## Phase 3 — RCL F3: preflight untrusted state before recursive parsing

- [x] Adapt RC's XML preflight to Lite's existing binary state format and 8 MiB
  limit; bound nesting and item count, reject DTD/entity declarations, validate
  length/magic/UTF-8 before JUCE XML and ValueTree recursion.
- [x] Route both host state and presets through that guarded entry point.
- [ ] Preserve parameter IDs and ordinary 1.x/2.0.0 state/preset compatibility.
- [x] Test excessive depth (including the 50,000-level audit payload), many
  attributes/elements, entities/DTD, malformed binary lengths, truncated input,
  embedded NUL/invalid UTF-8, null/oversized input and ordinary round trips.
- [x] Rejection leaves current parameters/pool unchanged and does not show
  startup modal dialogs or initiate sample loading.
- [x] Run engine tests and ASan/UBSan; use subprocess isolation for inputs that
  could crash the unfixed parser when verifying rejection.

**Acceptance:** previously crashing input is rejected safely; valid historical
state remains usable and rejected state cannot partially replace live state.

`Data/StateInput.h` now limits depth to 32 and items to 8,192 before recursive
parsing. Native tests reject the 350 KB / 50,000-level payload, DTDs/entities,
wide/attribute-heavy data, invalid UTF-8/NUL and bad binary framing without
changing serialized live state. The ordinary saved-state round trip passes;
real historical session/preset fixtures still need explicit host verification.

## Phase 4 — RC verification and shared prevention

RC changes belong in `round-robin-premium`, using its AGENTS.md, test scripts,
product identities and licensing/PACE configuration. Do not import Lite's
activation policy or release version into premium.

- [x] Apply the same export regression checker to RC's Mac build/CI and the
  prepared signed 1.0.2 artifacts, both slices.
- [ ] Check Windows exports using Windows tools when that platform is available.
- [x] Verify current source enforces visibility independently of build generator;
  do not rely only on the existing Xcode build's clean artifact.
- [ ] Audit RC's pool/pond/FS mutation APIs for the F2 lock boundary; add missing
  operation coverage if evidence shows a gap, rather than copying Lite's fix.
- [ ] Verify RC's existing InputParsing guard rejects the audit payload and that
  every host/preset path uses it.
- [ ] Verify actual distributed Windows Lite/RC source and bundles contain the
  artwork lifetime fix; test GUI close, explicit unload and host process exit.
- [ ] Review synchronous license bootstrap locking (F4) in both products. Design
  bounded/cancellable contention behavior and synthetic cross-process tests
  without weakening cache integrity, entitlement or offline usability.
- [x] Build RC separately with its own identity and run local engine/audio,
  licensing and native VST3 GUI validation.
- [x] Validate RC signed VST3/AU on native ARM64 and Intel, including GUI tests,
  with exact AU candidate identification and verified install restoration.
- [x] Run RC's own full unsigned AAX suite: 13 applicable checks pass.
- [x] Sign fresh RC staging with its own PACE configuration; verify Apple/PACE,
  correct product identity, resources and clean exports on both slices.
- [x] Run RC's own full AAX suite against that exact signed staging.
- [ ] Complete RC's exact-artifact retail-host/platform/signing checks before
  distributing a changed RC build. A new RC release is justified by verified
  changes, not automatically by the Lite version bump.

**Acceptance:** each shared finding has product-specific evidence: fixed/tested,
already protected, or a named pending item with its verification procedure.

RC prevention changes are limited to explicit language visibility, the same
product-independent checker, and checks in its existing ARM64/Intel CI jobs.
CI schedules/triggers are unchanged; no hosted run was requested. The isolated
`Builds/PT-Visibility-20261022` build uses RC's own 1.0.2 identity, minimum macOS
13, Moonbase/final ON, test manufacturer OFF and auto-copy OFF. All 35 plug-in
Objective-C++ compile commands carry hidden visibility. New VST3/AU/AAX outputs
pass all six format/slice export checks and retain required entry points;
minimum OS is 13.0 / SDK 26.5 in both slices. Native VST3 strictness 10 including
GUI tests passes; CTest engine/audio and licensing suites pass **2/2**. RC's
existing untrusted XML/JSON host/preset regressions execute successfully.
RC DSP/licensing/UI source was not modified by this prevention change.

Continuation: Rosetta execution is restored. New signed VST3/AU outputs pass
the same native/Intel checks recorded above. RC unsigned AAX passes **13/13**
in `rc-visibility-aax-unsigned/summary.json`; HDX cycle counts remain N/A.
Fresh `rc-visibility-signed/` staging uses RC's own Signing Only Wrap GUID
`8917B2B0-BED1-11F1-8E61-00505692C25A`. Independent PACE verification names
**Robin Control**, reports **signed, not wrapped**, and Apple verification
confirms Developer ID team `QS378YGT2W`, timestamp and hardened runtime.
Both-slice export gates pass for VST3/AU/AAX after signing.
`rc-visibility-aax-signed/summary.json` independently records **13/13 PASS**
against the exact signed RC executable; its hash was rechecked after the run.
These results use RC's own runner/descriptor and signing configuration. No RC
runtime source or activation policy was changed. The remaining host/platform/
customer/release checkpoints above still apply to a changed RC release.

Source review confirms RC's Add/Replace/Clear/Delete use `performPoolMutation`,
which takes the callback lock around the mutation/snapshots. Undo restores take
the same lock. This covers the specific unguarded Lite UI pattern, not every
possible RC concurrency scenario. Existing RC tests cover nested XML/JSON,
DTD/entity rejection and preservation of pool state for rejected host/preset
data; the new verification run exercises those existing regressions.

F4 remains a separate design/test item: both products acquire a synchronous
bootstrap cache lock, and SDK online validation can hold it over a network call.
Any replacement must preserve locally verified entitlement for headless/offline
rendering, atomic cache reads, deletion/revocation ordering, signed expiry/grace,
bounded lock cancellation and clean unload. A deferred retry that depends only
on the message loop can strand a headless render in an unlicensed state. Review
atomic snapshot storage or a bounded bootstrap plus message-loop-independent
retry with synthetic cross-process fixtures before changing production DRM.

## Phase 5 — Broader real-host regression matrix

Run on the final combined RCL candidate after F1–F3. Record platform/CPU/OS/host,
format, source/build identity, activation state, command/action, result and log.
Keep automated, manual, customer-reported and not-run results distinct.

| Scenario | Mac native | Mac Rosetta | Windows x64 |
| --- | --- | --- | --- |
| Native AAX cold scan and repeat startup | Pending | Pending | Pending |
| AAX editor repeated open/close and unload | Pending | Pending | Pending |
| Lite VST3 in PatchWork inside Pro Tools | Pending | Pending | Pending |
| Mono/stereo instrument tracks and MIDI/Trigger/Panic | Pending | Pending | Pending |
| Add/Replace/Delete/Clear while playing | Pending | Pending | Pending |
| Save/reopen, editor closed and multiple instances | Pending | Pending | Pending |
| Host rate/block-size changes and offline bounce | Pending | Pending | Pending |
| Activated/unactivated and synthetic network failures | Pending | Pending | Pending |
| Normal host exit and explicit plug-in unload | Pending | Pending | Pending |

If a row is inapplicable to an installed host configuration, record N/A with a
reason. Mac results do not establish Windows success. Do not change the real
machine clock or delete real licenses to simulate time/grace boundaries.

## Phase 6 — Prepare and release RCL 2.0.1

- [ ] Confirm accepted source contains F1–F3 and all required regressions.
- [ ] Set root/inner CMake, Projucer and relevant installer/release metadata to
  2.0.1; keep product/format identities and existing preset compatibility.
- [ ] Build fresh final-profile universal Mac and native Windows candidates.
- [ ] Engine/licensing tests, sanitizers, export checks, strictness-10 format
  validators and applicable unsigned/signed AAX checks pass for exact artifacts.
- [ ] Complete the host matrix above and record affected-customer acceptance.
- [ ] Sign using each product/platform's existing authorization; no customer
  iLok DRM. Package only freshly verified staging, exclude Standalone.
- [ ] Verify architecture/minimum OS, notices/build flags, page tables, payload
  hashes/modes/symlinks, installer signature and Mac notarization/stapling.
- [ ] Test installer upgrade/fresh install/uninstall on appropriate test setup,
  preserving user samples/presets/licenses. Confirm actual loaded hashes.
- [ ] Write release notes with verified behavior and remaining limitations.
- [ ] Review the complete package/evidence before any upload/publication.
- [ ] Publish/upload/update downloads only with the user's release instruction;
  keep superseded signed packages and rollback hashes available.

**Acceptance:** 2.0.1 is the package actually tested, not a relabelled 2.0.0
candidate. A release is not complete merely because signing/notarization passes.

## Phase 7 — Separate F6 sample-storage work

- [ ] Measure large/mixed-pool note-on allocation, copy time and peak memory.
- [ ] Design immutable shared sample ownership and non-audio-thread retirement.
- [ ] Define a product-appropriate total memory budget and user-facing errors.
- [ ] Test replacement/deletion/rate changes while old voices retain storage;
  verify no dangling pointers, audio-thread destruction or preset regressions.
- [ ] Validate and release this redesign separately from the focused crash fix.

## Reproduction commands for this Mac

Run from `/Users/alex/Documents/Github/robin-control-redesign`. Use a new build
and evidence suffix for a new candidate; the preserved F1 build must not be
rebuilt after F2/F3 source changes. The commands below reproduce the **combined**
candidate from current source, not the F1-only snapshot. The cache override is
local to this machine; omit it on a fresh clone to use the pinned download.

```sh
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  -S NewProject -B NewProject/build-pt-reliability-20261022 -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64' \
  -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk \
  -DCMAKE_C_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DCMAKE_OBJC_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_OBJCXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON \
  -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF -DRCL_BUILD_STANDALONE=OFF \
  -DFETCHCONTENT_SOURCE_DIR_MOONBASE_CPP=/Users/alex/Documents/Github/round-robin-premium/Builds/dependencies/moonbase-cpp-4.4.0
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  --build NewProject/build-pt-reliability-20261022 \
  --target RobinControlLite_VST3 RobinControlLite_AU RobinControlLite_AAX -j 4
python3 scripts/check-macos-exports.py \
  'NewProject/build-pt-reliability-20261022/RobinControlLite_artefacts/Release/VST3/Robin Control Lite.vst3' \
  'NewProject/build-pt-reliability-20261022/RobinControlLite_artefacts/Release/AU/Robin Control Lite.component' \
  'NewProject/build-pt-reliability-20261022/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin' \
  --expect-architecture arm64 --expect-architecture x86_64 \
  --output Releases/Testing/2.0.0/pro-tools-audit-20261022/reliability-exports.json
```

The fresh native Debug test build is
`NewProject/build-pt-reliability-tests-20261022`, configured with the same
toolchain/cache overrides, `CMAKE_BUILD_TYPE=Debug`,
`CMAKE_OSX_ARCHITECTURES=arm64`, `RCL_BUILD_TESTS=ON` and
`JUCE_AAX_SDK_PATH=/nonexistent`. It builds both test targets:

```sh
cmake --build NewProject/build-pt-reliability-tests-20261022 \
  --target RobinControlLiteTests RobinControlLiteLicensingTests -j 4
NewProject/build-pt-reliability-tests-20261022/tests/RobinControlLiteTests_artefacts/Debug/RobinControlLiteTests
NewProject/build-pt-reliability-tests-20261022/tests/RobinControlLiteLicensingTests_artefacts/Debug/RobinControlLiteLicensingTests
DEVELOPER_DIR=/Library/Developer/CommandLineTools \
RCL_MOONBASE_SOURCE=/Users/alex/Documents/Github/round-robin-premium/Builds/dependencies/moonbase-cpp-4.4.0 \
RCL_MACOS_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk \
RCL_SANITIZER_BUILD_ROOT=/Users/alex/Documents/Github/robin-control-redesign/NewProject/build-pt-sanitizers-20261022 \
  scripts/test-sanitizers.sh
```

Test executables need normal cache/GUI access; the sandbox can prevent creation
of temporary WAVs. The sanitizer runner's new optional build-root/sysroot
variables preserve its existing default behavior and avoid stale SDK caches.
The first sanitizer run was superseded after source/runner refinements; only
`reliability-sanitizers-final.log` is the conclusive current-source run. Do not
edit a running shell script or change its source inputs during validation.

Current RCL evidence: `reliability-artifact-manifest.json`,
`reliability-exports.json`, `reliability-build-final.log`,
`reliability-engine-tests.log`, `reliability-licensing-tests.log`,
`reliability-pluginval-native.log` and `reliability-sanitizers-final.log`.
ASan leak detection is disabled, as in the existing runner; this is not a leak
audit. The real licensed account skips the locked-processor assertion, while
synthetic entitlement/gate tests run. The malformed-MP3 fuzz case still emits
the same Debug assertion present in the pre-change baseline; all unit tests
pass, and this is not claimed as assertion-free decoding.

RC evidence is in the same Lite audit folder: `rc-visibility-configure.log`,
`rc-visibility-build.log`, `rc-visibility-exports.json`,
`rc-visibility-artifact-manifest.json`, `rc-visibility-pluginval-native.log` and
`rc-visibility-ctest.log`. RC's own detailed CTest log is under
`Builds/PT-Visibility-20261022/Testing/Temporary/LastTest.log` in premium.
Use `ctest --test-dir /Users/alex/Documents/Github/round-robin-premium/Builds/PT-Visibility-20261022 -C Release --output-on-failure`
to repeat those suites with normal cache/GUI access.

## Execution log

| Date | Step | Result / evidence |
| --- | --- | --- |
| 2026-10-22 | Baseline audit | Customer VST3 UUID and static-memory invalid free confirmed; six findings recorded in the crash audit. |
| 2026-10-22 | Plan opened | User authorized proceeding; focused F1 implementation and isolated candidate are next. |
| 2026-10-22 | F1 source/build | Explicit language visibility implemented; export checker added to Mac CI. Universal final-profile VST3/AU/AAX build and all six export checks pass. Original signed artifacts and installed bundles preserved. |
| 2026-10-22 | F1 native VST3 | Strictness 10 including GUI/editor-while-processing tests passes. Intel execution unavailable; some Avid tests abort on their bundled Intel Ruby. Full AAX/Intel/retail-host/customer gates remain pending. |
| 2026-10-22 | RCL F2/F3 | Processor-owned pool mutations and bounded state XML implemented; expanded native engine/licensing and ASan/UBSan/TSan pass. Combined universal Release formats retain clean exports; native strictness-10 GUI validation passes. |
| 2026-10-22 | RC prevention | Explicit language visibility and the common checker added to source/CI. New universal build, six export checks, native strictness-10 GUI validation and CTest 2/2 pass; installation OFF. |
| 2026-10-22 | Intel execution / signed VST3 | Installed Apple Rosetta 2; source/artifact hashes still match. Both signed products pass native/Intel strictness-10 GUI validation (4/4). |
| 2026-10-22 | RCL signed AAX | All 13 applicable checks pass before and after PACE signing; independent Apple/PACE verification and both-slice export gates pass. HDX cycle counts N/A. |
| 2026-10-22 | Exact signed AU validation | Both products pass native/Intel auval and strictness-10 GUI pluginval (8/8); failed registration attempts preserved separately. All prior user/system bundles restored and inventory-verified. |
| 2026-10-22 | RC AAX continuation | Own unsigned and signed suites each pass 13/13; RC-specific PACE signing and independent Apple/PACE/export checks pass. |
| 2026-10-22 | Automated Mac checkpoint | Both signed candidates pass their applicable automated format gates; metadata/hashes and restoration verified. Next: reversible retail Pro Tools / PatchWork testing and affected-customer retest. No hotfix published. |

When resuming: read this tracker, the crash audit, current git diff and latest
evidence first. Continue from the first pending dependency; do not overwrite old
signed staging, infer customer acceptance, or treat later phases as completed.
