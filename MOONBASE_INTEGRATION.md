# Robin Control Lite — Moonbase licensing

Started 2026-09-22. **Implemented for opt-in builds; not a published release.**
**Private beta update:** Alex subsequently requested a Moonbase-enabled beta
for upload/fresh-install testing. The signed, notarized universal macOS
VST3/AU/AAX installer and validation evidence are recorded in
`MOONBASE_BETA_1.md`. This named **Moonbase Beta 1** is separate from the earlier
DRM-free Beta 1 plan. It has not been uploaded or publicly released by the agent.

This is the macOS / Windows JUCE integration and validation handoff. Beta 1
remains DRM-free; final 2.0.0 requires Moonbase. PACE remains AAX digital
signing only, with no customer iLok DRM. Binary version and frozen APVTS
parameter IDs remain unchanged.

## Account and product configuration

Read from the signed-in [RCL product dashboard](https://app.moonbase.sh/products/robin-control-lite)
and its **Licensing → Implementation guide → C++ / JUCE** on 2026-09-22:

| Setting | Value |
|---|---|
| Tenant endpoint | `https://conduitdsp.moonbase.sh` |
| Product ID | `robin-control-lite` |
| License generator | Moonbase |
| Activations per license | 10 (server-controlled) |
| Offline activations | Enabled |
| Trials | Disabled |
| Product purchasable | Disabled |
| Release uploads | None at inspection |
| Online activation grace | **90 days**, Alex's latest instruction on 2026-09-22 |

These are observed settings, not changes made to the seller dashboard. The free
license acquisition/download flow still needs to be established before launch;
an activation integration alone does not grant every visitor a product license.

`NewProject/Source/Licensing/MoonbaseConfig.h` embeds the tenant's **public RSA
verification key** from the implementation guide. No seller API key, private
signing key, client secret, password, or customer token belongs in source, CI,
or the plugin. Updating/rotating the verification key requires a coordinated
client compatibility plan; do not silently regenerate it in Moonbase.

## Dependency and implementation

- Official [moonbase-cpp](https://github.com/Moonbase-sh/moonbase-cpp) **v4.4.0**,
  using its `moonbase_licensing` JUCE module. This is the correct repository.
- CMake FetchContent downloads the source archive with SHA-256
  `c3f2ea70dabb40a71cc0502fa07d82e18e2e6fdbd548165d2ec78272c327dede`.
  Only the JUCE module is added; the standalone core SDK's dependency build is skipped.
- Native crypto: Security.framework / IOKit on macOS, CNG/bcrypt on Windows.
  JUCE performs HTTP; no extra OpenSSL or CURL installation is required.
- `RCL_ENABLE_MOONBASE=OFF` is the Beta 1/development default. ON compiles the
  controller, UI, network support, and audio gate. `RCL_FINAL_RELEASE=ON`
  refuses configuration unless Moonbase is enabled; it is a configuration
  guard, not a replacement for checking the exact binaries being packaged.
- CMake is authoritative. The `.jucer` source list is mirrored but remains
  an unlicensed Beta 1 configuration. Use the CMake commands below for DRM builds.

`MoonbaseLicense` is retained by processors through a shared JUCE resource,
so editors can close without losing entitlement. Instances loaded from the same
plugin binary share state. Different formats/processes share the license file;
the SDK provides locking for online validation/update operations.

On construction, the cached token is verified locally for signature, product,
device, and expiry. Online tokens must also be within the configured grace
period. The unsigned outer JSON cache fields never grant access. This local
bootstrap allows existing activated projects to play without opening an editor
or waiting for the message loop/network. Missing or invalid licenses start silent.

The controller starts on the message thread and performs online work on SDK
workers. Every five minutes, when no activation/deactivation flow is in progress,
the service reloads and validates the license, including changes from another
format or host. Network failure is tolerated for up to **90 days since the last
successful online validation**. Signed token expiry or explicit revocation can
still end entitlement sooner. Permanently offline-activated licenses use local
validation and do not have the online grace period; Moonbase states they cannot
be remotely revoked or moved between machines. Forgetting their local file
does not release a server activation seat.

The final audio gate is after Tone processing and before the peak meter. MIDI,
Trigger, audition, and editor-closed/offline host renders pass through the same
gate. Only an atomic read and an 8 ms gain ramp run on the audio thread; there
is no licensing network, disk, crypto, or lock operation in `processBlock`.
Samples/presets remain intact, and license information is not stored in APVTS,
presets, or DAW state.

The editor shows a themed Moonbase activation panel when unlicensed and has a
License/Activate header button. It supports browser activation, offline request /
response files, license details and deactivation. Optional analytics, host/locale
metadata and update prompts are disabled. Licensing still sends the device
identifier/label and the SDK software User-Agent needed by its flow; “analytics
disabled” does not mean “no network traffic.”

License location (per OS user, shared across plugin formats):

- macOS: `~/Library/Conduit DSP/Robin Control Lite/license.mb`
  (`juce::File::userApplicationDataDirectory` is `~/Library` on macOS).
- Windows: `%APPDATA%\Conduit DSP\Robin Control Lite\license.mb`.
- SDK UI state: sibling `license.state.json` and SDK locking files as applicable.

The license contains customer identity and a signed token. Do not add it to Git,
logs, screenshots, test artifacts or support uploads. Synthetic test fixtures
use an unrelated public key and cannot unlock the production product.

## macOS — JUCE compilation

Run from this repository root. Use a separate build directory and disable
automatic copying so existing installed/signed candidates remain intact.

### Development / integration test

```sh
cmake -S NewProject -B NewProject/build-moonbase \
  -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug \
  -DRCL_ENABLE_MOONBASE=ON -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF \
  -DRCL_BUILD_STANDALONE=ON -DRCL_BUILD_TESTS=ON
cmake --build NewProject/build-moonbase --config Debug \
  --target RobinControlLite_VST3 RobinControlLite_AU RobinControlLite_AAX \
           RobinControlLite_Standalone RobinControlLiteLicensingTests -j 6
NewProject/build-moonbase/tests/RobinControlLiteLicensingTests_artefacts/Debug/RobinControlLiteLicensingTests
```

Explicit AAX target requires the SDK from `AAX_BUILD_AND_SIGNING.md`; omit the
target on a machine that is only validating VST3/AU. Standalone is local testing
only. Build products are under `NewProject/build-moonbase/RobinControlLite_artefacts/Debug/`.
To use an already-downloaded SDK without network access, configure with
`-DFETCHCONTENT_SOURCE_DIR_MOONBASE_CPP=/absolute/path/to/moonbase-cpp-4.4.0`.
That override bypasses FetchContent's archive check: verify its source/version.

### Final universal candidate

Use the CLT toolchain from `RELEASE_2.0.0.md`, preserving macOS 11 support:

```sh
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  -S NewProject -B NewProject/build-moonbase-release -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64' \
  -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk \
  -DCMAKE_C_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DCMAKE_OBJC_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang \
  -DCMAKE_OBJCXX_COMPILER=/Library/Developer/CommandLineTools/usr/bin/clang++ \
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON \
  -DRCL_BUILD_STANDALONE=OFF -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF
DEVELOPER_DIR=/Library/Developer/CommandLineTools cmake \
  --build NewProject/build-moonbase-release --config Release \
  --target RobinControlLite_VST3 RobinControlLite_AU RobinControlLite_AAX -j 6
```

Record compiler, SDK, JUCE revision, both architecture slices and the CMake
licensing flags with release evidence. Re-run pluginval/auval/AAX checks on the
new binaries. PACE-sign a **new** AAX staging copy and repeat retail Pro Tools
testing; today's earlier DRM-free AAX results do not cover these binaries.
Preserve AAX page XML, symlinks and `MoonbaseNotices.txt` during signing/packaging.
Do not overwrite signed staging with a rebuild. Notarization/packaging are
separate release steps, not performed by this integration.

## Windows — JUCE compilation

Read `WINDOWS_BUILD_AND_AAX.md` first. Clone the private authoritative repository
and use a fresh build directory with x64 MSVC and explicit local SDK paths.
Never copy a Mac CMake cache or binary to Windows. In PowerShell, adapt these
example SDK paths to the real machine:

```powershell
cmake -S NewProject -B NewProject/build-moonbase-win -A x64 `
  -DJUCE_PATH=C:/SDKs/JUCE -DJUCE_AAX_SDK_PATH=C:/SDKs/AAX `
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON `
  -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF -DRCL_BUILD_TESTS=ON
cmake --build NewProject/build-moonbase-win --config Release `
  --target RobinControlLite_VST3 RobinControlLiteLicensingTests
./NewProject/build-moonbase-win/tests/RobinControlLiteLicensingTests_artefacts/Release/RobinControlLiteLicensingTests.exe
```

VST3 is under `NewProject/build-moonbase-win/RobinControlLite_artefacts/Release/VST3/`.
There is no Windows AU. For Windows AAX, after establishing authorized SDK access:

```powershell
cmake --build NewProject/build-moonbase-win --config Release --target RobinControlLite_AAX
```

Windows AAX signing/validation is a separate pending workflow. Existing Windows
VST3 CI is not AAX evidence. Test real Windows networking, browser handoff,
offline file exchange, standard-user permissions, non-ASCII usernames/paths,
antivirus/firewall behavior, and Windows host reload. Do not infer a Windows
runtime pass from a successful macOS build.

## Automated coverage and current evidence

2026-09-22 local integration uses installed JUCE 8.0.15, native arm64 Debug.
These are development results, not final universal Release approval:

- [x] Moonbase ON: VST3, AU, AAX and Standalone compile/link on macOS.
- [x] Licensing regression executable: cached offline unlock before editor/message
  dispatch; missing, tampered, expired, wrong-product/device and stale-online
  rejection; forged outer-cache metadata cannot grant entitlement; gate ramps;
  actual processor's unlicensed MIDI/Trigger/audition output and meter are silent.
- [x] Moonbase OFF: existing engine/unit/fuzz suite passes.
- [x] Asynchronous startup, editor closure, offline deactivation and offline
  response import pass with synthetic signed tokens; no real customer seat is used.
- [x] Standalone activation UI opens and lays out in the 1400 × 400 editor.
- [x] Moonbase ON native arm64 Debug VST3: pluginval strictness 10, including GUI
  checks, reports SUCCESS. This does not replace licensed playback tests.
- [ ] End-to-end activation/deactivation against the real tenant, then restart.
- [ ] Offline file exchange with the real tenant and a disconnected machine.
- [ ] Exact licensed Release VST3/AU/AAX host validators and real-host smoke tests.
- [ ] Universal Release compilation and Intel/Rosetta runtime verification.
- [ ] Windows compilation/runtime, including non-ASCII profile paths.
- [ ] New GitHub CI runs: matrix now builds Moonbase ON/OFF on macOS/Windows;
  licensing regression tests run in the macOS test job and Windows ON job.

Local logs, build settings and artifact hashes are saved in the ignored
`Releases/Testing/moonbase-integration-20260922/` directory. They do not transfer
to Windows through Git. Installed AU/VST3 and the PACE-signed AAX candidate were
not replaced by this work.

The existing `RobinControlLiteTests` intentionally compiles with DRM OFF so
audio assertions still exercise DSP. `RobinControlLiteLicensingTests` links
the actual DRM-enabled plugin code. Sanitizer scripts still cover the engine
test target; do not claim licensing-specific sanitizer coverage yet.

## Release checkpoints

- [ ] Confirm the customer path for receiving a free Moonbase RCL license;
  test existing-user migration as well as a new customer.
- [ ] Verify activated playback after saving/reopening a DAW project with the
  editor closed, including offline bounce and several simultaneous instances.
- [ ] Verify deactivation/revocation and offline grace within/beyond 90 days;
  do not change the whole system clock on the production workstation to test it.
- [ ] Test browser cancellation/timeouts, corrupt cache recovery, offline response
  for the wrong device, network outage, and license file permission failures.
- [ ] Test UI open/close during activation and host unload with requests running.
- [ ] Review the EULA: its current §1 allows any number of computers, whereas the
  dashboard has 10 seats, and §7 says the plugin is entirely offline. Reconcile
  these before distributing DRM builds; the existing published EULA was not rewritten here.
- [ ] Update the canonical website privacy/download documentation for Moonbase.
  The repository Privacy.md now distinguishes public/Beta 1 from DRM development.
- [ ] Ensure `MoonbaseNotices.txt` accompanies all distributed plugin bundles.
- [ ] Record `RCL_ENABLE_MOONBASE=ON`, `RCL_FINAL_RELEASE=ON` and hashes of the
  exact artifacts used for final signing, validation and installer assembly.

## Primary references

- [JUCE integration documentation](https://moonbase.sh/docs/licensing/sdks/juce/)
- [Pinned module guide](https://github.com/Moonbase-sh/moonbase-cpp/blob/v4.4.0/docs/juce-module.md)
- [SDK security scope](https://github.com/Moonbase-sh/moonbase-cpp/blob/v4.4.0/docs/security.md)
- [Product configuration](https://app.moonbase.sh/products/robin-control-lite)

The SDK supplies cryptographic license verification and activation, not a
guarantee against binary patching. Keep that distinction when describing DRM.
