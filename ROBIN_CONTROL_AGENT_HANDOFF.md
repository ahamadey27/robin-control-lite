# Robin Control agent handoff from Lite v2.0.0

Updated 2026-09-22. This document is in the authoritative **Robin Control Lite**
repository, `ahamadey27/robin-control-lite`. Premium **Robin Control** is a separate
product/repository at `/Users/alex/Documents/Github/round-robin-premium` on this Mac.
Read that repository's AGENTS.md, current code and release records before edits.
This handoff does not claim that premium has received these changes.

## Current direction

- Moonbase is Alex's selected licensing provider for Lite and future products.
  The older July custom Cloudflare/license-file proposal in company context is
  superseded as a provider choice. Do not restart that implementation.
- PACE is for **AAX digital signing only**. Do not introduce customer iLok DRM.
- Lite v2.0.0 final uses Moonbase 4.4.0 plus reviewed local corrections, JUCE
  8.0.15, C++17, and a processor-level output gate shared across instances of
  each binary. Closing the editor must not disable licensing enforcement or
  interrupt a valid entitlement. No licensing work runs in the audio callback.
- Lite's 90-day online grace, 10 activations, permanent offline activation and
  disabled trials are **Lite policy**. Confirm premium's product settings;
  a paid product may need a different trial or fulfillment configuration.

## Reuse deliberately

Start with `MOONBASE_INTEGRATION.md`, `FINAL_RELEASE_2.0.0.md`,
`WINDOWS_BUILD_AND_AAX.md`, `AAX_BUILD_AND_SIGNING.md`, and `TESTING.md`.
Useful implementation references:

- `NewProject/Source/Licensing/`: configuration, shared service, activation UI,
  cached signed-token verification and the final output ramp/gate.
- `NewProject/cmake/MoonbaseHardening.cmake`: apply checked corrections to a
  **build-local copy** of the pinned SDK. Includes UTF-8 Windows filesystem paths,
  locking of startup reads, bounded cache parsing and rejection of future
  validation times. Re-review each patch on SDK upgrades.
- `tests/licensing_tests.cpp` and synthetic `tests/licensing-fixtures/`: prove
  wrong product/device, invalid signature, expiry, grace, corrupt cache, output
  gating without an editor, asynchronous startup and offline import/deactivate.
  Test keys/tokens are synthetic, not production credentials.
- `SampleSlot` / `SampleLoader` and engine regressions: bound decoded allocations,
  reject non-finite audio, respect interpolator input length, serialize sample-rate
  changes, and bound state parsing. Port only where premium has equivalent paths.
- CMake final-release guard, embedded `RCLBuildConfig.txt`, and macOS installer
  checks prevent accidentally packaging DRM-free development binaries.

**Never copy Lite's Moonbase product ID, plug-in codes, bundle ID, activation
storage path, AAX type IDs/page table or PACE Wrap GUID into Robin Control.**
Retrieve premium's own Moonbase product configuration. The tenant public RSA key
is not a seller API secret, but still verify it against that product's guide.
No seller/private signing credentials or customer license tokens belong in Git.

Preserve premium's existing parameter IDs, presets, DSP behavior and product
scope. Lite's monophonic playback and dormant envelope/effect plumbing are not
instructions to remove premium features. Respect each product's callback locks.

## Platform and release evidence

Build x64 Windows in a fresh MSVC directory; macOS artifacts and caches do not
transfer. AU is macOS-only. Windows AAX signing/validation requires its own setup
and retail Pro Tools test. Use the installed Windows tool documentation, not
macOS paths or Apple certificate names. PACE `sign`, never `wrap`; verify both
PACE and platform signatures, then validate the exact signed output.

Build macOS arm64+x86_64, preserve macOS 11 support where that product requires
it, and sign only fresh staging. Do not rebuild over signed bundles. Include
license notices, page tables and PACE symlinks in packaging. Signed installers
must be notarized and stapled; compare extracted payload hashes with staging.

A sanitizer or pluginval pass is not a manual licensed-audio/session-recall test.
Test missing license, online activation, restart/editor closed, multiple instances,
network outage, permanent offline exchange, deactivation and host unload. Never
change the production Mac's date or delete real licenses for automated tests.

## Delivery context

Lite's private Moonbase beta uses dashboard release 1.0.1 despite binary version
2.0.0. An accidental ACTIVE 1.0.0 dashboard release also exists. Those labels do
not change binary versioning. Moonbase delivery email/download was tested; the
fulfillment text still contains the beta link and needs replacement before final
launch. Do not reuse it for premium. Hosted portal is enabled; public self-service
acquisition/migration and website policies remain launch tasks. No final upload,
publication, customer email, or premium changes are authorized by this handoff.
