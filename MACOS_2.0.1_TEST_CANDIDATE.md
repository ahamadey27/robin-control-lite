# Robin Control Lite 2.0.1 — macOS test candidate

Started October 23, 2026 at Alex's request. This is a full, unpublished macOS
test installer for the Pro Tools crash correction. Public Lite remains 2.0.0.
Follow `PRO_TOOLS_RELIABILITY_PLAN_2026-10-22.md` for the underlying findings
and diagnostic host evidence; this record does not replace customer acceptance.

**Build result: all scoped automated macOS gates pass.** The complete test
installer is signed, notarized, stapled and accepted by Gatekeeper. It remains
unpublished. The build/validation workflow preserved prior installations and
2.0.0 release/diagnostic executables. Alex subsequently installed the package
and confirms repeating the PatchWork test successfully. All three installed
system bundles independently match the signed 2.0.1 payloads, including hashes,
modes and symlinks. Pro Tools was already closed, so the actual wrapper-loaded
path during that reported test was not independently observed. Evidence:
`local-package-acceptance.json`. Do not request the completed local test again
as though no user result exists; affected-customer acceptance remains separate.

## Deliverable

- Installer: `Releases/Installers/Robin Control Lite 2.0.1.pkg`
- Size: **16,332,733 bytes**.
- Final SHA-256: `db90cc547e83b141bc46a985db77db510306255915dfc517c5095957f0afae5b`.
- Adjacent `.pkg.sha256` file records the same checksum.
- At Alex's request, the package/checksum were moved into the standard installer
  directory alongside 2.0.0. Contents and checksum are unchanged; this file move
  does not publish a release. `artifact-relocation.json` preserves the old path.
- Apple notarization: `91471210-5995-489c-a443-ae205523bb4f`, **Accepted**.
- Signed staging: `Releases/Testing/2.0.1/macos-candidate-20261023/candidate-signed/`.
- Consolidated evidence: `final-candidate-summary.json` in that evidence run.

## Scope and identity

- Universal arm64/x86_64 VST3, AU and AAX; minimum macOS 11.0.
- Version 2.0.1 in root/authoritative CMake, Projucer and generated bundle/package
  metadata. Product, bundle, manufacturer, format and parameter IDs unchanged.
- Moonbase ON and FINAL_RELEASE ON, existing 90-day online grace and separate
  permanent offline activation policy retained; analytics disabled.
- AAX uses Lite's existing PACE Signing Only configuration, without customer
  iLok DRM. Standalone excluded; automatic installation disabled.
- Runtime sources match the combined diagnostic fix at `65dc245`: language
  visibility, locked pool-operation APIs and bounded state XML preflight.
  No additional DSP, UI, licensing or sample-storage redesign is included.

## Reproduction and evidence

Fresh build: `NewProject/build-release-2.0.1-20261023`.
Fresh ignored evidence: `Releases/Testing/2.0.1/macos-candidate-20261023/`.
Signed staging is separate from build outputs. Existing signed
2.0.0 releases, F1-only and combined diagnostic builds must remain unchanged.

Use the Command Line Tools compiler and installed macOS 26.5 SDK. The historical
15.4 SDK is unavailable; do not copy its obsolete path from the 2.0.0 record.
JUCE remains pinned to 8.0.15, commit
`91ad83ae34a81e0833b1a2b0866f54846370ae53`. The pinned Moonbase 4.4.0 archive and
all 148 cached files were verified before reuse; hardening is applied only in
the new build-local module copy. Configure/build commands and tool results are
recorded in the evidence directory. `source-manifest.json` records the base
commit plus exact source hashes; version changes are not identified as an
already committed revision.

## Package checkpoints

- [x] Version metadata synchronized; plug-in IDs and parameter IDs retained.
- [x] Fresh final-profile universal configure; all 35 plug-in Objective-C++
  compile commands carry hidden visibility.
- [x] All three universal formats build with expected resources and metadata.
- [x] Engine/licensing tests and ASan/UBSan/TSan pass on the 2.0.1 source.
- [x] Both-slice internal-export and host-entry-point checks pass before signing.
- [x] Unsigned AAX passes all 13 applicable Native tests; HDX cycle counts N/A.
- [x] Fresh Developer ID/PACE signed staging independently verifies.
- [x] Signed VST3 and AU pass native/Intel strictness-10 GUI validation, with
  conclusive candidate identity and restoration of temporary AU registration.
- [x] Signed AAX passes all 13 applicable Native tests; both-slice exports pass.
- [x] Signed installer payloads match staged files, modes and symlink targets;
  installer version/destinations/EULA/resources and signature verified.
- [x] Apple notarization accepted, ticket stapled, Gatekeeper assessment passes.
- [x] Existing installed bundles and earlier signed releases preserved.

The first user-scope AU attempt selected the original system copy in native
auval and mixed paths in native pluginval; those results remain inconclusive.
The completed matched-copy run used inventory-verified candidate bytes at both
registry paths. All four native/Intel checks pass, followed by independent
restoration inventory/signature checks for both prior AUs. No AU caches or
preferences were deleted. Both signed VST3 runs explicitly execute Editor,
Open editor whilst processing and Editor Automation. Sanitizer leak detection
is disabled, as in the existing runner; no leak-audit claim is made.

Two initial package-checker assertions assumed the wrong location for Apple's
non-relocation flag and omitted AAX's nested PACE signature bundle. The checker
was corrected to inspect the actual metadata; all final payload hashes/modes/
symlinks match staging, including a fresh extraction after stapling. These
checker attempts are preserved and are not classified as plug-in failures.

## Remaining acceptance

Local 2.0.0 diagnostic host passes do not establish exact 2.0.1 package acceptance.
The affected customer's VST3/PatchWork retest is the next crash-specific gate;
their PatchWork version and separate startup-crash report remain outstanding.
The local package installation and PatchWork retest are user-confirmed. Remaining
fresh-install/uninstall and broader host playback/restore checks, historical
state/preset fixtures, broader activation/network/host/platform coverage and
Windows builds retain their own status. Actual Mach-O unload was not measured
by the manual insert removal/reinsertion checks. F4–F6 follow-up work is separate
from this focused candidate.

No public upload, download replacement, tag, customer message, commerce change
or release publication is authorized by building/notarizing this installer.
