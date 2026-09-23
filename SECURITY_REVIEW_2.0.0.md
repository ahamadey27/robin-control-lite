# Robin Control Lite 2.0.0 security and robustness review

2026-09-22. Scope: local C++ source, pinned JUCE/Moonbase integration, state/audio
input handling, licensing tests, build configuration and macOS distribution.
This is a focused source review and automated test pass, not a penetration-test
certification or a claim that the plugin contains no vulnerabilities.

## Corrections in this release

| Area | Finding and correction |
| --- | --- |
| Sample decoding | Bound decoded allocation before reading: <=50 million floats across channels, <=64 channels, finite 8–192 kHz source rate. Check decoder results and reject NaN/Inf data before DSP. |
| Resampling | Use the bounded JUCE interpolation overload to prevent reads past short/tail buffers; check resampled length before integer conversion/allocation and preserve at least one frame. |
| Shared sample state | Lock sample-rate mutation and metadata snapshots; reject a load if the playback rate changes before its prepared buffer can be committed. |
| Host/preset input | Reject null, negative/empty and >8 MiB state; bound preset files before loading; ignore repeated/out-of-range slots, unexpected node types and relative sample paths. |
| License cache | Lock startup reads across processes to avoid partial-write recovery deleting a valid license. Reject >1 MiB cache JSON before parsing. Signature/product/device/expiry validation remains mandatory. |
| Windows license storage | Convert JUCE UTF-8 paths with `std::filesystem::u8path`; exercise Unicode cache paths in tests. Actual Windows execution remains pending. |
| Clock behavior | Negative validation ages cannot suppress SDK online checks or extend online grace. This does not make a client clock tamper-proof. |
| Release mix-ups | Final CMake requires DRM; signed resources record build flags; final macOS packaging refuses artifacts without final+Moonbase markers, notices and valid signatures. |
| Dependency parity | JUCE fallback now uses the tested 8.0.15 commit and archive checksum. Moonbase remains checksum-pinned 4.4.0; local patches require exact source context and pass repeated configure testing. |

The sample limit bounds an individual decode, not total host memory. Large pools
can still consume substantial RAM. If a host sample-rate change would push a
loaded sample over the resampled limit, that slot is cleared; do not advertise
unlimited file size/duration. These checks do not sandbox third-party decoders.

## Validation and dependency review

- Engine/processor regressions and synthetic signed-license tests passed under
  ASan+UBSan and TSan on native arm64 macOS. Leak detection was disabled in ASan;
  do not describe this as a leak audit. Tests include actual locked processor
  MIDI/Trigger/audition output, startup/editor-independent entitlement, invalid
  tokens, corrupt/oversized caches, Unicode paths and offline response lifecycle.
- Final format validation, architecture/signature checks and installer results
  are recorded in `FINAL_RELEASE_2.0.0.md`, with raw logs under the ignored
  `Releases/Testing/2.0.0/final-20260922/` directory.
- A tracked-file pattern scan for private keys, GitHub/AWS access tokens and JWTs
  found only the five intentional synthetic JWT fixtures. This limited scan is
  not exhaustive credential detection. No production license token was copied
  into the release evidence or repository by this review.
- [JUCE's published advisory page](https://github.com/juce-framework/JUCE/security/advisories)
  showed no published advisories at review time. Absence of advisories is not
  proof of absence of defects. Moonbase's advisory page could not be retrieved;
  no comprehensive CVE clearance is claimed.
- Reviewed the [pinned Moonbase security scope](https://github.com/Moonbase-sh/moonbase-cpp/blob/v4.4.0/docs/security.md)
  and [JUCE integration guide](https://moonbase.sh/docs/licensing/sdks/juce/).
  Verification uses signed claims and the configured public key; local cache
  metadata alone cannot grant entitlement. Client-side DRM can still be patched
  by an attacker; no obfuscation/anti-debug guarantee is made.

## Remaining release checks

Windows compilation/runtime and AAX signing are separate pending evidence.
Complete licensed real-host playback/session recall and a real permanent offline
activation exchange before public launch. The 90-day online grace has synthetic
boundary coverage, not 90 days of elapsed real-world testing. Sanitizers cover
exercised paths; they cannot prove all races, parser defects or SDK failures absent.

Repository EULA/Privacy text now explains Moonbase. Public website policy and
free-license delivery configuration must be reconciled before public release.
