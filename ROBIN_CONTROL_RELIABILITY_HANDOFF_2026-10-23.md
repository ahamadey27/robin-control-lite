# Robin Control — applicability of the Lite crash fixes

Reviewed October 23, 2026 at Alex's request. Premium checkout:
`/Users/alex/Documents/Github/round-robin-premium`, branch
`agent/windows-vst3-ci`, HEAD `706fef9`, source version **1.0.2**.
Read that repository's `AGENTS.md`, `TESTING.md`, existing release records and
`PRO_TOOLS_RELIABILITY_PLAN_2026-10-22.md` before changing or building it.
The master reliability tracker remains in canonical Lite. This document is
mirrored into premium for its next agent. This review changes documentation
only; no Robin Control runtime, version, installed plug-in or package changed.

## Answer for the next agent

**The lessons apply, but do not paste Lite's implementation over Robin Control.**
Robin Control already has the principal protections. The explicit F1 visibility
and export regression gate were applied to RC during the October 22 work; F2
transaction locking and F3 guarded parsing already existed. RC also already
has the Windows artwork lifetime pattern that Lite still needs to audit.

**Do not restart the investigation or repeat completed tests on unchanged,
exactly identified RC artifacts.** Reuse the evidence below while source and
binary hashes match. Complete the remaining RC host/product coverage. Any new
runtime/dependency/build-profile change requires its appropriate regressions;
a newly built/signed/versioned release package needs its own exact-artifact
validation and package checks. Lite's host pass cannot stand in for RC's.

## Findings mapped to Robin Control

| Finding | What changed in Lite | Robin Control status and action |
| --- | --- | --- |
| F1: cross-module JUCE symbols | Hide C/C++/Objective-C/Objective-C++ implementation symbols for shared code and every format; add all-slice export/entry-point gate. | **Already applied** in `NewProject/CMakeLists.txt` and Mac CI, committed as `529b680`. Its previously examined prepared 1.0.2 Mac binaries already had clean exports. Preserve the explicit generator-independent guard and `scripts/check-macos-exports.py`; no automatic emergency RC release follows from the Lite finding. |
| F2: pool edits racing playback | Processor-owned Add/Replace/Delete/Clear transactions; decode outside the lock, commit slots/lists/sounds together; add operation regressions. | **Existing RC transaction architecture:** `performPoolMutation` / `performPondMutation` take the callback lock around snapshots and mutation; Pool/Pond editor actions use them. `SamplePond` self-locks mutators, and Undo restore/state swap paths lock. Preserve all five banks and Undo. Audit/extend actual Add/Replace/Delete/Clear operation coverage under concurrent playback if completing F2; existing generic concurrency loops do not establish every operation/bank combination. |
| F3: recursive state/preset parsing | Bounded binary XML preflight before JUCE recursion, through both host and preset entry points. | **Already protected:** `Data/InputParsing.h` bounds depth 32/items 8192, rejects DTD/entities, validates binary framing/UTF-8. Processor host restore and legacy/portable preset XML/JSON use it. Existing regression suite rejects nested/DTD/malformed host and preset inputs without replacing the pool. Keep RC's larger embedded-audio limits; do not copy Lite's 8 MiB cap. The existing RC nested regression is 10,000 levels, not a claimed exact 50,000-level audit-payload run. |
| F4: synchronous license-store contention | Investigated, **not fixed** by Lite 2.0.1. | **Still separate:** RC `MoonbaseLicense::restoreCachedLicense` takes a store lock during construction. Async activation does not remove that bootstrap wait. A future bounded/cancellable design must preserve signed entitlement, revocation ordering, offline/headless rendering and safe teardown. Do not transplant Lite licensing policy or implement a casual async-only workaround. |
| F5: Windows graphics lifetime | Identified; the Mac Lite source still has static/default-backend artwork. Windows provenance/native testing remains needed. | **RC already has the safer pattern:** editor-owned `SharedResourcePointer<BrandImages>` with software image conversion; preferences are also editor-owned. Preserve this. Actual distributed Windows source/artifacts and GUI/remove/exit outcomes still need product/platform-specific provenance. This was not the supplied Mac invalid-free stack. |
| F6: note-on copies/allocation | Identified, **not fixed** by Lite 2.0.1. | Keep separate from the focused crash fix. Measure before redesigning immutable sample storage/non-audio-thread retirement, with RC's Pool/Ponds/FS/Undo/preset lifetime requirements. Do not remove protective copies without ownership guarantees. |

The customer dump captures **Lite VST3 editor construction inside PatchWork in
Pro Tools**, freeing JUCE's static `emptyString`. Export leakage strongly supports
symbol interposition as the leading diagnosis. It is not a dump proving an RC
crash, nor evidence that the separately mentioned startup incident has the same
cause. Affected-customer acceptance and separate startup evidence remain pending.

## Existing RC evidence that can be reused

Isolated production-profile build: `Builds/PT-Visibility-20261022` in premium.
Version 1.0.2, minimum macOS 13.0, SDK 26.5, both x86_64/arm64, Moonbase ON,
FINAL_RELEASE ON, TEST_BUILD OFF, auto-copy OFF. Do not overwrite this build or
older signed outputs while evaluating a new candidate.

Fresh signed RC staging and consolidated logs are in the Lite repository's
ignored `Releases/Testing/2.0.0/pro-tools-audit-20261022/`:

- `rc-visibility-signed/`: RC VST3/AU/AAX signed diagnostic bundles.
- `rc-visibility-artifact-manifest.json`: source/build identity and initial
  prevention-source hashes; all 35 Objective-C++ commands have hidden visibility.
- `rc-visibility-vst-au-signed-summary.json` and
  `rc-visibility-aax-signed-summary.json`: signed hashes/resources/identity.
- `signed-vst3-validation-summary.json`: **two RC native/Intel strictness-10
  GUI passes**; do not count Lite's two runs as RC results.
- `combined-au-isolated-validation/summary.json`, product `rc`: **four exact RC
  AU checks pass**, native/Intel auval plus strictness-10 GUI pluginval. Prior
  user/system AUs were restored and independently verified. That combined attempt
  has an overall failure due to Lite's registration problem; RC's separate
  four-pass result remains valid and must not be relabelled as an RC failure.
- `rc-visibility-aax-unsigned/summary.json` and
  `rc-visibility-aax-signed/summary.json`: **13/13 applicable Native checks pass
  before and after RC's own PACE signing**; DSP/HDX cycle test N/A.
- Signed all-format/both-slice export gates, Apple verification and PACE Signing
  Only verification pass. PACE names **Robin Control**, not Lite.
- `Builds/PT-Visibility-20261022/Testing/Temporary/LastTest.log` in premium:
  **2/2 CTest engine/audio and licensing passes** on this prevention build.

October 23 recheck: the three signed RC executables still match those recorded
hashes; prevention-source hashes still match, and the diff from `529b680` to
current HEAD is empty for CMake/runtime/tests/export checker/CI. Later commits
in that range are handoff progress, not new RC runtime fixes. No rerun was
performed merely to repeat those same results. Ignored Mac evidence and signed
bundles do not transfer with a Windows clone; the target agent must distinguish
unavailable local evidence from a requirement to repeat account onboarding.

## Focused next steps, rather than repeating all Lite work

1. **Close only the specific code-coverage gaps.** Review actual mutation API
   calls across Pool, Pond 1/2 and FS Left/Right, including Add/Replace/Delete/
   Clear and Undo/Redo while MIDI/audio, automation and state I/O run. Extend
   the existing test harness where a gap is verified. The current concurrency
   switch covers state, pool swap/insert/audition/reset/Trigger/Panic; it is not
   an exhaustive race test of every UI bank transaction. RC already has nested
   XML/JSON and rejected-state preservation tests; add an exact audit payload
   only as a focused regression, without replacing its parser architecture.
2. **RC real-host smoke test on an identified candidate.** In retail Pro Tools,
   check native AAX and VST3 inside PatchWork: cold load, repeated editor open/
   close, MIDI, Trigger/Panic, insert remove/reinsert and clean host quit/reopen.
   Verify the loaded RC path/hash/UUID. Lite's manual results are not RC results;
   insert removal is not proof of actual Mach-O unloading.
3. **RC-specific behavior, once.** Exercise Pool plus both Ponds, Probability /
   Insert/Layer, FS white/black routing, Series/Random, editing while playing,
   Undo/Redo, Auto-Chop Apply, and `.rr` / session restoration including embedded
   chop audio. A compact representative pass is the next useful manual check;
   this review does not declare every historical RC manual release gate closed.
4. **Only if producing a changed RC release:** choose its own version, build
   fresh staging, rerun engine/audio/licensing and relevant sanitizers, exact
   signed native/Intel validators/export gates and unsigned/signed AAX checks;
   compare installer payloads and sign/notarize/staple/assess the new package.
   Reuse the existing commands and lessons; do not re-investigate solved setup
   issues. Windows requires native builds/signatures/validators/host checks of
   its own. Existing compliance/release records retain their status.

There is no reason to bump RC to Lite's version number, copy Lite's package,
repeat Lite's entire manual matrix under RC's name, or issue a new RC hotfix
solely because Lite 2.0.1 exists. Decide RC release scope from verified RC changes
and its own acceptance, not the sibling's version bump.

## Preserve product separation

RC target/product `RobinControl` / `Robin Control`, codes `Cdsp` / `Rctl`, bundle
`dsp.conduit.RobinControl`, minimum macOS **13**. Keep its parameter IDs, three-
voice normal routing, two-voice FS behavior, five banks, `.rr` embedded audio,
Auto-Chop, Undo and Factory content. Do not impose Lite's monophonic/20-slot
scope or its state byte limit on premium.

RC PACE Signing Only Wrap GUID:
**`8917B2B0-BED1-11F1-8E61-00505692C25A`**. Never use Lite's GUID/page tables.
RC's Moonbase product is `robin-control`; current source uses 90-day purchased-
online grace, 30-day trials and permanent offline activation disabled. Lite's
10-activation/permanent-offline policy is not permission to change premium.
Use premium's own final-profile/manufacturer guards and signing workflow.

## Bounds of this handoff

This is a source/evidence comparison, not a new full RC concurrency audit or a
new host validation run. No runtime patch, release version, signing, installation,
customer message, upload, or publication was performed. Preserve the unrelated
untracked `generated_images/` folder and other existing working-tree changes.
