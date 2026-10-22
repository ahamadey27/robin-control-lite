# RCL and RC — Pro Tools reliability execution plan

Started October 22, 2026. Owner: Alex / conduit.dsp. Update this file after each
verified checkpoint; an unchecked item is pending, not an implied failure.

## Objective and current checkpoint

Resolve the Lite 2.0.0 interface crash reported inside Pro Tools / Blue Cat's
PatchWork, investigate the separately reported startup crash, and prevent the
same build and lifecycle risks in Robin Control premium.

**Current checkpoint: automated macOS validation passes for both products'
signed diagnostic candidates; RCL native AAX and VST3-in-PatchWork smoke tests
pass in retail Pro Tools on this Mac. Customer/remaining host, platform and
release gates remain open.** Pool
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
The exact RCL candidate was cold-scanned by ARM64 Pro Tools 26.4.1.179 and
instantiated successfully; Alex confirmed mono/stereo editor cycles, sample
loading, Trigger and Panic worked without a crash. Alex also confirmed that the
saved session reopened with both Lite tracks and their samples restored.
The temporary AAX test is complete: the original installed bundle has been
restored and its complete file/mode/symlink inventory plus Apple/PACE signatures
verified. The corrected Lite VST3 also passes the requested PatchWork Synth
2.78 playback, editor-cycle and remove/reload sequence, confirmed by Alex and
live mapped-path/inode/hash/UUID evidence. MIDI playback, Add/Replace/Delete/
Clear/reload during playback, Series/Random, and about one minute of editor-
closed playback followed by reopen also pass (Alex's manual confirmation).
Wrapper session restoration also passes: Alex confirmed samples, order,
parameter settings and MIDI playback return after a full host quit/reopen;
the fresh process maps the same identified candidate. The 48 kHz offline WAV
render passes file/decode inspection and Alex confirms it sounds normal.
MIDI playback at 64 and 512 samples also passes (user-confirmed), corroborated
by Avid buffer-change records and return to 256. An extra 128-sample setting
change was observed but is not separately claimed as a playback test.
The actual **48 → 96 kHz session switch and 96 kHz offline render now pass**:
regular Pro Tools' trace records 96,000 Hz / 256 samples, the exact corrected
VST3 remains mapped, and the new stereo/24-bit/96 kHz WAV decodes with signal.
Alex confirms the corrected session and bounce succeeded. The first misnamed
48 kHz attempt is preserved separately; it is not counted as 96 kHz evidence.
`host-validation-checkpoint-summary.json` records this local Mac milestone and
the remaining customer/native-host/Windows/licensing/package/release gates.
This covers the tested local setup;
the combined candidate and newer host/OS/wrapper do not isolate which change
prevents the customer's crash.
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
- [x] Retail Pro Tools: scan/launch, instantiate native AAX, open/close editor
  repeatedly on mono/stereo instrument tracks.
- [x] PatchWork inside Pro Tools: load candidate VST3, open/close editor, remove
  and reload; record PatchWork and host versions and exact loaded binary UUID.
  Removal/reload is user-confirmed; actual Mach-O unloading was not measured.
- [x] Test with the normal installed plug-in collection; restore temporary AAX.
- [ ] Use a reduced collection if needed for load-order diagnosis after a
  reproducible failure. No reduced-collection result is claimed.
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

### October 22 native retail host checkpoint

RC work was committed locally as `529b680` on `agent/windows-vst3-ci`, including
the export safeguards, investigation and this tracker. The unrelated
`generated_images/` folder was excluded. No push or hosted CI run was performed.

The first unprivileged AAX move was denied before any bundle moved; its record
verifies that the original remained intact. The subsequent administrator swap
installed the exact signed RCL diagnostic AAX and verified every file hash,
mode and symlink plus independent Apple/PACE signatures. Evidence is in
`pro-tools-native-host-admin-20261022/`; `install-state.json` records the original
inventory, verified preservation copy and protected whole-bundle backup at
`/Library/Application Support/Avid/Audio/Conduit-RCL-Host-Test-20261022/Robin Control Lite.aaxplugin`.
The local evidence helper `test-retail-aax-privileged.py restore` subsequently
completed with Pro Tools closed. The original bundle is back at its installed
path; its full file/mode/symlink inventory and independent Apple/PACE signatures
pass. The protected backup directory was removed after the move, and a copy of
the tested installed candidate remains in evidence. `restoration-verification.json`
records a separate inventory check. The temporary AAX swap is finished.

Observed: retail Pro Tools **26.4.1.179**, **ARM64**, macOS **27.0.1**, normal
installed AAX collection. Avid's startup trace regenerates the Lite AAX cache
and reaches a new session without crashing. Native-control timeouts during
startup were not host crashes; the application subsequently became ready.
`loaded-aax-identity.json` records the loaded path/inode and executable SHA-256
`e8530e7784491f78e84b5af347f41b26421564fc90b2e0b1cb334ba8d4f351c1`, matching
signed staging. The process sample records ARM64 and the actual host version;
the matching ARM64 plug-in UUID is `3BBFF262-DB0B-3E37-A892-0079AD3A45B9`.
The editor was observed on an AAX Native instrument insert.

**User-confirmed:** Alex reported both mono/stereo instrument tracks worked
without a crash after the requested repeated editor open/close cycles (about
10 each), sample loading, Trigger and Panic checks. These are manual results,
not an automated event count or independent listening measurement. The session
is `~/Documents/RCL AAX Reliability 20261022/RCL AAX Reliability 20261022.ptx`.
Alex separately confirmed: **saved session reopened with both Lite tracks and
samples restored**. The second startup trace is `Pro_Tools_2026_10_22_18_26_00.txt`;
the diagnostic AAX remained installed until the subsequent restoration. No live
module mapping was captured for this second launch. This is a manual session-
restoration pass, not a PatchWork VST3 result. MIDI playback, explicit unload and
the remaining Phase 5 scenarios retain their separate status.

PatchWork was initially absent from both system/user AAX trees. The official
**2.78** demo was downloaded and inspected read-only; its installer verifies as
**Blue Cat Audio (FR28HR7442)** with trusted Apple notarization.
`patchwork-demo-preflight.json` records artifact hashes and both AAX identities.
Alex handled the installer locally. Both `BC PatchWork AAX.aaxplugin` and
`BC PatchWork Synth AAX.aaxplugin` are now present as version 2.78, and the
second Avid startup trace records scanning them. Agent acceptance of third-party
terms or agent installation is not claimed.
The [official product page](https://www.bluecataudio.com/Products/Product_PatchWork/)
identifies 2.78 and describes changes to VST3 editor loading; earlier 2.76 notes
also mention editor/unload fixes. The customer's actual wrapper version remains
unknown; these release notes do not establish the customer's root cause.
For convenient manual loading, a complete inventory-matching copy of the new
signed Lite VST3 is prepared at:

`/private/tmp/RCL-PatchWork-Test-20261022/Robin Control Lite.vst3`

`patchwork-manual-candidate.json` records its source, signature verification,
SHA-256 `d8edf3cbd5165c8788a9a4524389e6adf798dd573fae8c3a905b393f4232d821`
and expected ARM64 UUID `7C9E0BEA-2949-3E07-905E-9F9F418E89F6`.
The installed system VST3 was not replaced. Select this explicit temporary path
with PatchWork's **Load VST3** command; the same version label on another copy
cannot establish its identity. Requested manual sequence: PatchWork Synth on a
stereo instrument track, load the candidate, load samples, Trigger/Panic and
about ten editor cycles, then remove/reload and try playback. Leave the host
open for a live mapped-path/inode/hash check. This requested sequence now
**passes (user-confirmed)**. Alex confirmed that the VST3 loaded into PatchWork
and all requested checks worked.

`pro-tools-native-host-admin-20261022/wrapper-pass-summary.json` records Pro Tools
PID 55415 mapping the exact temporary VST3 path, inode **156115002**, matching
executable SHA-256 and ARM64 UUID above, alongside **BC PatchWork Synth AAX
2.78**. A brief Apple process sample independently confirms **ARM64 Pro Tools
26.4.1.179 on macOS 27.0.1**. The restored original Lite AAX is also mapped in
this process; it is not the diagnostic native AAX. The tested corrected VST3
coexisted successfully with that original AAX and the normal plug-in collection.
Do not infer exact editor cycle counts or physical module unloading from the
manual remove/reload result. The customer used Pro Tools 24.6 / macOS 14.2 with
an unknown PatchWork version and an effect-wrapper module in the dump; this
Synth 2.78 pass does not establish acceptance on that configuration.

A restricted-sandbox signature check returned an invalid-signature error even
though the copy's complete inventory and executable hash matched signed staging.
Independent verification with access to macOS signing services passed. Preserve
the distinction; no re-signing or binary modification was needed.

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

October 22 Windows source/artifact readiness check: Lite's current
`PluginEditor.cpp` still has the function-static `logo` and default-backend keyed/
output images; RC's editor-owned `SharedResourcePointer<BrandImages>` and
`SoftwareImageType` conversion remain present. `windows-f5-readiness.json` records
source hashes and local installer hashes. Only Lite 1.0.0 was found in the
checked local Windows installer locations; RC 1.0.2 is present in Downloads.
These names/hashes do not establish the public Windows build's source or actual
GUI/unload behavior. No Windows binary was executed, no artwork was changed,
and the distributed-source/lifetime checkpoint remains open.

## Phase 5 — Broader real-host regression matrix

Run on the final combined RCL candidate after F1–F3. Record platform/CPU/OS/host,
format, source/build identity, activation state, command/action, result and log.
Keep automated, manual, customer-reported and not-run results distinct.

| Scenario | Mac native | Mac Rosetta | Windows x64 |
| --- | --- | --- | --- |
| Native AAX cold scan and repeat startup | Cold scan observed PASS; saved-session reopen PASS (user) | Pending | Pending |
| AAX editor repeated open/close and unload | Editor cycles PASS (user); unload pending | Pending | Pending |
| Lite VST3 in PatchWork inside Pro Tools | PASS (user + mapped identity), PatchWork Synth 2.78 | Pending | Pending |
| Native AAX mono/stereo instrument tracks and Trigger/Panic | PASS (user); native AAX MIDI coverage pending | Pending | Pending |
| Wrapper MIDI playback and Series/Random | PASS (user), corrected VST3 in PatchWork | Pending | Pending |
| Add/Replace/Delete/Clear while playing | PASS (user), corrected VST3 in PatchWork | Pending | Pending |
| Save/reopen, editor closed and multiple instances | Native AAX two instances/session restore PASS; wrapper editor-closed playback and full session/sample/settings/MIDI restore PASS (user + fresh-process identity) | Pending | Pending |
| Wrapper session sample-rate changes | PASS: actual 48 → 96 kHz session switch at 256 samples, correct VST3 + offline render verified | Pending | Pending |
| Wrapper host buffer-size changes | PASS at 64/512 (user + trace), restored 256 at 48 kHz; 128 setting change observed only | Pending | Pending |
| Wrapper offline bounce | 48 kHz PASS (user listening + trace/WAV); corrected 96 kHz render PASS (user report + actual host/Offline trace + WAV) | Pending | Pending |
| Activated/unactivated and synthetic network failures | Pending | Pending | Pending |
| Normal host exit and explicit plug-in unload | Wrapper remove/reload PASS (user); native AAX unload/exit assessment pending | Pending | Pending |

If a row is inapplicable to an installed host configuration, record N/A with a
reason. Mac results do not establish Windows success. Do not change the real
machine clock or delete real licenses to simulate time/grace boundaries.

### Documented Mac wrapper regression run — corrected VST3

Use the disposable test session and the already verified temporary VST3 above.
The system native AAX is now the original release; further candidate-native AAX
checks require a separately recorded reversible install or a later identified
2.0.1 installer. Do not attribute tests of the restored AAX to the candidate.
Keep the temporary VST3 available until saved wrapper sessions are finished;
`/private/tmp` is not a permanent plug-in installation or archival location.

Run preparation (October 22): the corrected candidate still maps in Pro Tools
PID 55415 alongside PatchWork Synth. Avid trace
`Pro_Tools_2026_10_22_18_31_07.txt` records an initial **48 kHz / 256-sample** engine
configuration; later rate/block changes need their own evidence. The fixture
`/private/tmp/RCL-PatchWork-Test-20261022/RCL-MIDI-120bpm-16bars.mid` contains
16 bars of quarter-note triggers at 120 BPM (SMF type 0, MIDI note 60, velocity
80). Import it onto the wrapper instrument track or use an existing disposable
MIDI loop. `mac-manual-regression-20261022/results.json` tracks every requested
operation separately. The first four steps now pass (user-confirmed): MIDI,
all requested pool operations, Series/Random, editor-closed playback/reopen and
full wrapper-session restoration.
`playback-pool-pass-mappings.json` records the still-live PID 55415 mapping the
same temporary VST3 inode **156115002** and expected signed SHA-256 alongside
PatchWork Synth. Alex subsequently confirmed the full wrapper restoration step
passes. `wrapper-restoration-mappings.json` records a new Pro Tools PID **67404**
(previously 55415) with the same VST3 mapped path/inode/hash and ARM64 UUID.
`wrapper-restoration-process-sample.txt` independently confirms ARM64 Pro Tools
26.4.1.179 / macOS 27.0.1. The saved disposable `.ptx` changed before the new
launch; its metadata/hash are retained privately. Avid's new
`Pro_Tools_2026_10_22_18_52_23.txt` trace still records **48 kHz / 256 samples**.
The manual result covers samples, order, two parameter settings and MIDI playback;
it is not an automated state-field comparison. Later evidence confirms the
buffer changes and completed offline render described below. Playback at 64/512
and WAV listening are now user-confirmed PASS. The corrected actual 96 kHz
host-session switch and offline render also pass with rate/identity/WAV evidence
below; the earlier setup mismatch remains historical. The prepared
`/private/tmp/RCL-PatchWork-Test-20261022/Bounces/` folder can hold later test
exports. Creating fixtures/folders does not establish a playback/bounce pass.

1. On the PatchWork Synth instrument track, loop a short MIDI region with notes
   that trigger Lite. Verify actual MIDI playback separately from the Trigger
   button; record the host rate/block size and output behavior.
2. While the loop runs, exercise additive **Load Samples**, Replace, Delete and
   Clear, then reload the pool. Try Series and Random. Clearing intentionally
   leaves an empty/silent pool; check for crashes, freezes and corrupted order.
   Use small disposable samples and preserve existing personal sample files.
3. Close Lite's editor and let MIDI playback continue for about a minute. Reopen
   it and confirm that playback and controls still work.
4. Save the disposable session with the wrapper/sample pool, quit Pro Tools and
   reopen it. Confirm the VST3 still loads from the identified temporary path,
   and that samples, order, parameter settings and playback restore. The earlier
   native AAX restore pass does not cover this wrapper-state test.
5. If the playback engine supports it, test another sample rate (for example
   96 kHz) and small/large block sizes, recording the actual values and any
   unsupported choices. Use a copy of the disposable session where necessary.
6. Bounce a short section **offline** to WAV; listen to it and compare its
   audible behavior with live playback. Random mode need not produce identical
   sample choices. Record missing/truncated audio, crashes or freezes.

Completed 48 kHz procedure: stop playback before selecting **64** and
**512** samples in **Setup → Playback Engine**, test MIDI at each, then return
to **256**; report actual alternatives if those values are unavailable.
For the bounce, solo the PatchWork instrument track, select a short passage
including sample tails, then use **File → Bounce Mix**, **Offline** enabled,
WAV/interleaved/24-bit/48 kHz. Save
`/private/tmp/RCL-PatchWork-Test-20261022/Bounces/RCL-offline-48k.wav` and listen
for missing/truncated/distorted audio. Soloing identifies the wrapper output
separately from the restored original native-AAX tracks. The separate session-
sample-rate check was subsequently completed with actual 96 kHz host-rate
and 96 kHz output evidence below; a different export rate alone is insufficient.
[Avid buffer instructions](https://www.avid.com/pro-tools/user-guide/change-buffer-size)
and [Bounce Mix instructions](https://kb.avid.com/pkb/articles/en_US/Knowledge/Bounce-Mix-a-Session).

October 22 bounce evidence: Alex reports the WAV bounced with no errors.
The actual default filename is
`/private/tmp/RCL-PatchWork-Test-20261022/Bounces/RCL AAX Reliability 20261022.wav`;
no rename was required. Inspection confirms **48,000 Hz, 2 channels, 24-bit PCM,
32.362646 seconds**, successful complete decode, peak **−9.282601 dBFS** and RMS
**−32.577570 dBFS**. The file contains signal; this does not establish expected
musical content or tail integrity. SHA-256:
`62e0e4fc0ef08f641b66d796a3ab9294c77807fc236c640950d29e636d64b025`.
An identical verified copy is preserved as
`mac-manual-regression-20261022/offline-48k-output.wav`, with ffprobe/astats and
`offline-48k-file-inspection.json` alongside it. These raw artifacts stay ignored.
Avid's trace explicitly records offline bounce preparation/completion/finalization;
`offline-bounce-trace-extract.txt` preserves those lines. The trace also records
**64 → 512 → 128 → 256** hardware-buffer changes at 48 kHz. Live mappings still
match the corrected VST3 and PatchWork Synth. Alex explicitly confirmed:
**the WAV and MIDI playback at both 64 and 512 samples sounded normal**.
Those manual fields are now PASS. The trace's extra 128-sample setting change
is retained as an observation, not a separately confirmed playback test.

The higher-rate procedure uses an actual **96 kHz session copy** via **File → Save
Copy In** (called Save Session Copy In in Avid's guide), then open that copy.
Changing only the bounce export rate does not change the host rate. Preserve
the original 48 kHz disposable session. Verify actual host rate via Session
Setup/Avid trace, play the same MIDI with the restored pool/settings and listen
for changed pitch, shortened/missing tails or a freeze/crash. If supported,
make a separate 96 kHz offline WAV for inspection. The corrected host-session/
render run is now verified below. Originally suggested copy name/location:
`/private/tmp/RCL-PatchWork-Test-20261022/Sessions/RCL-96k-Reliability`.
Use a 256-sample buffer, verify Session Setup really shows 96 kHz, then test
MIDI pitch/duration/tails and editor close/reopen. Save the separate Offline
WAV as `RCL-offline-96k.wav` in the existing Bounces folder, listen, and leave
the 96 kHz copy open for actual engine-rate and candidate-identity verification. [Avid sample-rate instructions](https://www.avid.com/pro-tools/user-guide/change-sample-rate-bit-depth).

First purported 96 kHz run — verification mismatch: Alex reported the test
passed and the bounced audio sounded good, but `RCL-offline-96k.wav` has **48,000
Hz**, stereo 24-bit PCM and **32.362646 s** duration. It decodes successfully;
this is another valid 48 kHz output, not higher-rate evidence. Its SHA-256 is
`b6bf8940b822e9bceffa858c65f465db5f0d982ff04312021c028f1915aa4ac8`.
A verified preservation copy, metadata, decode log and summary are saved as
`mac-manual-regression-20261022/rate-96k-attempt-48k-*`.
`rate-96k-host-context.json` still shows 48 kHz and the same correct candidate.
A read-only native UI inspection confirmed the active window/path was the
original `RCL AAX Reliability 20261022` session in Documents.

The actual saved copy was
`/private/tmp/RCL-PatchWork-Test-20261022/Sessions/RCL-96k-Reliabilit.ptx`
(the filename lacks the final `y`). Initial native Pro Tools/Finder navigation
did not confirm a switch away from the original session; the copy was selected
in Finder. Alex reported the copied session was open at 96 kHz, but a subsequent
direct native **Setup → Session** inspection in regular Pro Tools still showed
**Sample Rate: 48 kHz** in the original session. This discrepancy is retained;
the report alone does not override the observed rate. Alex then asked how to
set 96 kHz, and received the following corrected procedure:

1. Stop playback; choose **File → Save Session Copy In…** in regular Pro Tools.
2. Explicitly set the dialog's **Sample Rate dropdown to 96 kHz**.
3. Save a distinct new copy named **RCL-96k-Test**; preserve the existing files.
4. Choose **File → Open → Session…** and open that new `.ptx`. Saving the copy
   leaves the original open; do not infer a switch from completing the save.
5. Check **Setup → Session → Sample Rate: 96 kHz**, then repeat MIDI pitch/
   duration/tails and editor close/reopen at a 256-sample buffer.
6. In **Bounce Mix**, explicitly select **96 kHz** as the export rate too, enable
   Offline, write a separate WAV and listen. Leave that session open for evidence.

Native-control attempts to choose Open Session were unreliable (one CUA timeout);
no session switch was claimed. The user is operating the controls. Brief Developer
startup logs also appeared during Finder-open attempts but ended before plug-in
collection; no Developer process or new Pro Tools diagnostic report was found.
They are not format validation or a reproduction of the customer crash.
The copy's rate is not inferred from its name. This is a test-setup discrepancy,
not a reproduced plug-in failure. It did not close the higher-rate checkpoint;
the corrected run below supplies the actual host/output-rate evidence.

**Corrected 96 kHz run — host/session/render verified.** Alex subsequently
confirmed the session was at 96 kHz and successfully bounced. Regular Pro Tools
PID **67404** opened the same `RCL-96k-Reliabilit.ptx` path after the copy was
recreated/configured. The Avid trace records `PBEngine: sample rate = 96000`,
`current buffer size 256`, `CAAXAutomationManager::SetSampleRate ... 96000` and
completed Offline bounce preparation/finalization after that session switch.
Live path/inode/hash/ARM64 UUID still match the corrected signed VST3. Evidence:
`rate-96k-final-host-context.json` and `offline-96k-file-inspection.json`.

The corrected `RCL-offline-96k.wav` is **96,000 Hz, 2 channels, 24-bit PCM,
32.362646 seconds**; complete decode/astats pass with peak **−9.210014 dBFS**
and RMS **−32.507355 dBFS**. Its duration matches the 48 kHz export (same timeline
selection); that alone is not an automatic musical/tail comparison. SHA-256:
`6854b04a13cd04fef56885988a61dd1676b84545d5737aaab0cfe49478a40570`.
A verified copy is preserved as `mac-manual-regression-20261022/offline-96k-output.wav`.
The earlier 48 kHz file with the same temporary filename remains preserved under
`rate-96k-attempt-48k-output.wav`. Do not confuse their hashes. The final report
confirms the corrected session/render succeeded; the earlier explicit “sounds
good” report preceded the rate correction, and no automated audio inspection
can establish expected pitch/timbre/tail behavior. Exact release-artifact host/
listening checks remain part of the final package gate.

`host-validation-checkpoint-summary.json` captures the completed documented
Mac wrapper subset, not completion of Phase 5 or public-release approval.
Remaining work includes affected-customer acceptance/separate startup evidence,
historical-state fixtures, native AAX MIDI/unload scenarios, live licensing/
contention checks, Windows/F5 provenance and native tests, RC host acceptance,
and fresh versioned/signed/notarized 2.0.1 package verification. Publication
still requires the user's release instruction. Keep the diagnostic staging
unchanged while preparing any later candidate in a new build directory.

Record each result separately. The operator's pass is manual evidence; a fresh
live path/inode/hash check after reopen should confirm the same candidate.
Remaining RC, Rosetta-host, Windows, activation/network, historical-session and
affected-customer checks retain their own status. No publication is authorized
by completing this local run.

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
| 2026-10-22 | RC commit | Local `529b680` records the reviewed changes and validation; unrelated generated images excluded, no push. |
| 2026-10-22 | Native retail AAX | Exact signed RCL candidate loaded in ARM64 Pro Tools 26.4.1.179; cold startup observed PASS. Alex confirms mono/stereo editor cycles, sample loading, Trigger and Panic PASS. Original AAX subsequently restored and inventory/signature-verified. |
| 2026-10-22 | Session restore / AAX rollback | Alex confirms both saved Lite tracks and samples restore. Original system AAX restored with complete inventory and Apple/PACE verification; protected backup removed. |
| 2026-10-22 | Wrapper preparation | PatchWork AAX/Synth 2.78 installed locally by Alex and scanned by Pro Tools. Exact signed Lite VST3 copied to the short temporary path and verified; wrapper loading/cycles/unload test pending. |
| 2026-10-22 | PatchWork local pass | Alex confirms corrected VST3 playback, editor cycles and remove/reload PASS. Live mapped path/inode/hash/UUID matches signed candidate; PatchWork Synth 2.78 and ARM64 Pro Tools 26.4.1.179 / macOS 27.0.1 verified. Customer configuration/retest remains pending. |
| 2026-10-22 | Remaining Mac run started | Exact wrapper candidate still mapped; MIDI fixture and per-operation result record prepared. Initial Avid engine trace is 48 kHz / 256 samples. Playback/pool/editor-closed results requested; pending. |
| 2026-10-22 | Windows F5 readiness | Static default-backend logo remains in Lite source; RC has editor-owned software artwork. Local installer/source hashes recorded; actual distributed Windows source and unload tests remain pending. |
| 2026-10-22 | MIDI/pool/editor-closed pass | Alex reports no errors for MIDI playback, Add/Replace/Delete/Clear/reload during playback, Series/Random and a minute of editor-closed playback/reopen. Live candidate mapping/hash still matches. Wrapper save/reopen requested; pending. |
| 2026-10-22 | Wrapper restoration pass | Alex confirms samples/order/settings/MIDI restore after full quit/reopen. New ARM64 Pro Tools PID 67404 maps the same corrected VST3 inode/hash/UUID; trace remains 48 kHz / 256. Buffer-size and offline-bounce checks requested; rate check remains separate. |
| 2026-10-22 | 48 kHz bounce inspected | Alex reports error-free WAV bounce. Avid confirms offline rendering and 64/512/128 → 256 buffer changes; corrected candidate still mapped. Stereo 24-bit/48 kHz/32.36 s WAV decodes with signal (peak −9.28 dBFS); preserved/hash-verified. Listening/buffer-playback confirmation requested; 96 kHz session run pending. |
| 2026-10-22 | 48 kHz listening/buffer pass | Alex explicitly confirms normal WAV listening and MIDI playback at both 64/512 samples. Those fields now PASS; 128 remains only an observed setting change. Actual 96 kHz session copy/playback/editor/bounce requested, not yet verified. |
| 2026-10-22 | 96 kHz verification mismatch | User reports normal test/bounce, but named 96k WAV is 48 kHz and original session remained active. Output preserved/decoded/hash-verified. Saved session copy selected in Finder; operator checking actual Session Setup rate. Higher-rate checkpoint remains pending. |
| 2026-10-22 | Higher-rate setup clarified | User's 96 kHz report was not corroborated: direct regular-host Session Setup showed 48 kHz. Explicit Sample Rate dropdown/save/new-session-open and export-rate steps supplied for RCL-96k-Test. User handles controls after unreliable native Open Session automation; no higher-rate pass claimed. |
| 2026-10-22 | Corrected 96 kHz run verified | Regular-host trace proves actual 96 kHz / 256 session switch and Offline render; same corrected VST3 mapped. New stereo/24-bit/96 kHz/32.36 s WAV decodes with signal and is preserved/hash-verified. User confirms corrected session/render success; earlier setup mismatch retained separately. Broader release gates remain open. |

When resuming: read this tracker, the crash audit, current git diff and latest
evidence first. Continue from the first pending dependency; do not overwrite old
signed staging, infer customer acceptance, or treat later phases as completed.
