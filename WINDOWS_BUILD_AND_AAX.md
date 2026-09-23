# Windows clone, build, and AAX handoff

Last updated: 2026-09-22. Product: **Robin Control Lite 2.0.0**.

Read this on the Windows machine before configuring. These are preparation
instructions derived from the repository and tool documentation, **not a record
of a successful Windows 2.0.0 AAX build**. The macOS AAX has been signed, validated,
installed, and reported working in regular Pro Tools / Intro. Windows AAX build,
signing, validation, and retail-host acceptance remain to be established.

## 1. Clone the right repository

Use the private `ahamadey27/robin-control-lite` repository. The Mac folder name
`robin-control-redesign` is historical; `round-robin-lite` is a retired sibling.
Authenticate Git to GitHub using your normal credential manager, not a token
embedded in the clone URL.

From a suitable parent directory in PowerShell:

```powershell
git clone https://github.com/ahamadey27/robin-control-lite.git
if ($LASTEXITCODE -ne 0) { throw "Clone failed" }
Set-Location robin-control-lite
git status -sb
git log -1 --oneline
```

Before moving machines, commit and push the intended source/docs on the Mac or
explicitly transfer the intended branch. A clone receives only pushed commits,
not uncommitted files. Match the source revision being tested and record it.
Do not assume an existing remote-tracking ref proves the newest local edits are
on GitHub. The final Mac build was based on `68bc4ba` plus the local audit changes; use
the later commit containing the entire final handoff and code fixes. The older
`957654d` AAX implementation alone is not a final Moonbase release.

Read `AGENTS.md`, `FINAL_RELEASE_2.0.0.md`, `AAX_BUILD_AND_SIGNING.md`, and `TESTING.md`.
Mac `/Users/alex/...` paths are provenance, not Windows paths to create. If the
company website repository is also cloned, its sibling `CONDUIT_DSP_CONTEXT.md`
provides business context; its absence does not block this build.

Git does **not** bring over JUCE/AAX SDK installations, PACE tools or credentials,
developer certificates, the physical signing iLok, compiled bundles, CMake caches,
or the ignored `Releases/Testing/` logs. Set up the Windows machine and generate
new evidence. Do not copy a Mac build directory or `.aaxplugin` to Windows.

## 2. Install prerequisites and set explicit SDK paths

- Git, CMake **3.22 or newer**, and Visual Studio 2022 / Build Tools with
  **Desktop development with C++**, the MSVC v143 x64 toolset, and a Windows SDK.
  The commands below use the `Visual Studio 17 2022` generator and `-A x64`.
  For another Visual Studio version, verify the installed CMake generator first.
- A Windows-compatible **AAX SDK 2.9.0** installation from the authorized Avid
  account, including `Interfaces/AAX.h` and the required library sources/tools.
- For signing/testing: current Windows iLok License Manager, authorized Windows
  PACE AAX Code Signing Tools, the developer signing iLok USB, Windows DigiShell /
  AAX Validator, and regular retail Pro Tools / Intro. Developer Pro Tools may
  help debug unsigned builds but cannot establish retail signature acceptance.
- Python 3 if adapting the validator runner; Windows pluginval for VST3 checks.

For parity with the successful Mac build, use JUCE **8.0.15**, exact commit
`91ad83ae34a81e0833b1a2b0866f54846370ae53`. The FetchContent fallback now downloads this exact **8.0.15** revision with a
SHA-256 check. An explicitly supplied JUCE checkout must match it. Moonbase
4.4.0 is also checksum-pinned; CMake applies the reviewed build-local corrections
in `NewProject/cmake/MoonbaseHardening.cmake`. Do not bypass those corrections.

Example local paths (adjust to where you actually installed the SDK):

```powershell
$rclSdkRoot = Join-Path $env:USERPROFILE "SDKs"
$rclJucePath = Join-Path $rclSdkRoot "JUCE"
$rclAaxSdk = Join-Path $rclSdkRoot "aax-sdk-2-9-0"
New-Item -ItemType Directory -Force -Path $rclSdkRoot | Out-Null

# Only clone into a new directory. Preserve any existing JUCE working tree.
git clone https://github.com/juce-framework/JUCE.git "$rclJucePath"
if ($LASTEXITCODE -ne 0) { throw "JUCE clone failed; inspect any existing checkout" }
git -C "$rclJucePath" checkout 91ad83ae34a81e0833b1a2b0866f54846370ae53
if ($LASTEXITCODE -ne 0) { throw "JUCE checkout failed" }

# Unpack the authorized AAX SDK to $rclAaxSdk before continuing.
if (-not (Test-Path "$rclAaxSdk/Interfaces/AAX.h")) { throw "AAX SDK not found" }
if (-not (Test-Path "$rclJucePath/CMakeLists.txt")) { throw "JUCE not found" }
```

Always pass both SDK paths explicitly. The repository's default paths use the
`HOME` environment variable, which may be absent or different on Windows.
Do not change `HOME` to work around that. SDKs stay outside the repository.

## 3. Configure and compile x64 Release

Run from the repository root in the same PowerShell session as the path setup.
Use a fresh build directory; do not reuse a cache from macOS or another generator.

```powershell
cmake -S NewProject -B NewProject/build-windows-aax -G "Visual Studio 17 2022" -A x64 `
  "-DJUCE_PATH=$rclJucePath" "-DJUCE_AAX_SDK_PATH=$rclAaxSdk" `
  -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=ON `
  -DRCL_BUILD_STANDALONE=OFF -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF -DRCL_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

cmake --build NewProject/build-windows-aax --config Release `
  --target RobinControlLite_VST3 RobinControlLite_AAX RobinControlLiteTests RobinControlLiteLicensingTests --parallel 4
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

& ./NewProject/build-windows-aax/tests/RobinControlLiteTests_artefacts/Release/RobinControlLiteTests.exe
if ($LASTEXITCODE -ne 0) { throw "Unit/processor tests failed" }
& ./NewProject/build-windows-aax/tests/RobinControlLiteLicensingTests_artefacts/Release/RobinControlLiteLicensingTests.exe
if ($LASTEXITCODE -ne 0) { throw "Licensing tests failed" }
```

The configure output must report **AAX SDK found ... enabling AAX format**.
Explicitly requesting `RobinControlLite_AAX` makes a missing format fail instead
of silently producing only VST3. JUCE omits AU on Windows. Standalone is optional
for local debugging and is not part of public releases.

Visual Studio is a multi-configuration generator: `--config Release` chooses the
configuration. `CMAKE_BUILD_TYPE` does not select it. In PowerShell, a continuation
backtick must be the last character on its line.

Expected outputs under
`NewProject/build-windows-aax/RobinControlLite_artefacts/Release/`:

| Item | Relative path |
| --- | --- |
| VST3 bundle | `VST3/Robin Control Lite.vst3/` |
| AAX bundle | `AAX/Robin Control Lite.aaxplugin/` |
| AAX x64 binary | `AAX/Robin Control Lite.aaxplugin/Contents/x64/Robin Control Lite.aaxplugin` |
| AAX page table | `AAX/Robin Control Lite.aaxplugin/Contents/Resources/RobinControlLitePages.xml` |

Check these paths on the actual build. The outer `.aaxplugin` is a directory;
do not distribute just its inner x64 binary. CMake's Windows page-table copy
rule was added with the Mac fix but still needs its first Windows verification.
Check version 2.0.0, x64 architecture, and resource contents before signing.

## 4. Establish Windows signing and validation

The existing scripts have platform limits:

| Script | Windows status |
| --- | --- |
| `scripts/setup-pace-signing.py` | macOS-only tool path and Keychain setup; do not run unchanged |
| `scripts/test-aax-native.py` | macOS executable path, SDK path, and POSIX process handling; port/test before using on Windows |
| `scripts/test-plugin.sh` / `scripts/test-sanitizers.sh` | macOS workflows; not Windows AAX evidence |
| `.github/workflows/validate.yml` | Existing Windows VST3 build/pluginval job; no AAX SDK, signing, or AAX validation |

For the first Windows AAX pass:

1. Locate the actual installed `wraptool.exe` and `dsh.exe`. Read their version
   and `help` output; do not substitute guessed Mac paths or the obsolete
   `-e validator-batch=...` command. Record versions and Windows source commit.
2. Authenticate privately using Windows PACE's documented credential caching
   flow. macOS Keychain credentials do not arrive with the clone. The already
   provisioned developer signing iLok and Lite configuration are reusable where
   authorized; a missing Windows cache is not evidence that a new product or
   signing certificate is needed. Do not place passwords in scripts or Git.
3. Establish Windows platform signing requirements from that installed tool's
   documentation. No Windows Authenticode certificate has been verified in this
   session. The current PACE help describes Windows `--signid` as a certificate
   thumbprint; the Mac Developer ID string is **not** a Windows signing identity.
   Confirm the applicable certificate/signing arrangement before issuing a
   command; do not claim this machine is signing-ready from the Mac results.
4. Port the validator runner or invoke Windows DigiShell manually. Discover
   tests with `listtests`, inspect the actual descriptor, and run **every
   applicable** test before and after signing. Confirm the Windows toolkit's
   definition of `test.cycle_counts` and Native-only descriptor before recording
   that DSP/HDX check as N/A. Never suppress other failures or trust exit 0 / the
   last PASS alone. Preserve stdout, helper logs, and a per-test summary.
5. Sign a separate staged **Windows build**, using explicit PACE **`sign`**, Lite's
   verified Wrap GUID `8F95C7F0-B538-11F1-8437-00505692AD3E`, and the established
   Windows signing options. Never use `wrap` or enable PACE customer DRM.
   Verify PACE's output identifies Conduit DSP LLC / Robin Control Lite and says
   **signed, not wrapped**. Verify the platform signature as required by the
   selected Windows signing workflow. Do not alter the signed payload afterward.
6. Run Windows pluginval strictness 10 on the VST3 separately. That pass does not
   validate AAX. See the existing Windows CI job for its pluginval invocation.

For an individual diagnostic after assigning the verified Windows DigiShell
path to `$rclDsh`, the documented stdin interface is:

```powershell
$rclAaxBundle = (Resolve-Path "NewProject/build-windows-aax/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin").Path
@(
  "load_dish aaxval"
  "listtests"
  "runtest [test.page_table.load, `"$rclAaxBundle`"]"
  "exit"
) | & $rclDsh
```

This example runs only the page-table diagnostic, **not the full release gate**.
Confirm the installed Windows toolkit accepts the same interface. For signed
testing, point it at signed staging. DigiShell takes quoted paths with spaces
but does not support character escaping inside its own command syntax.

## 5. Install, test in retail Pro Tools, and record evidence

Close Pro Tools, preserve any old installed Lite bundle outside its scan path,
and install the complete signed bundle to:

```text
C:\Program Files\Common Files\Avid\Audio\Plug-Ins\Robin Control Lite.aaxplugin
```

Use the **64-bit** Common Files path. Elevation may be needed. Preserve the entire
bundle layout/signing resources, compare its hashes with the validated signed
staging, and verify the installed signature. Test in regular Pro Tools / Intro
on Windows: scan/open, mono/stereo Instrument tracks, sample load/audition, MIDI,
Trigger/Panic, UI resize/reopen, automation, preset/session save/restore, and the
absence of Lite iLok activation. Pro Tools' own licensing is separate. Include
customer-environment testing without developer signing entitlements.

Record Windows/Pro Tools/tool versions, source/JUCE commits, build commands,
binary/resource hashes, signing/validation results, and specific manual outcomes
in the release checklist. Keep raw artifacts/logs under an ignored evidence
directory and summarize reproducible findings in tracked Markdown. Extend the
Windows installer/uninstaller for AAX only after its signed payload is validated;
Mac notarization and a Mac `.pkg` do not produce a Windows release.

## Licensing and scope

**Final 2.0.0 requires Moonbase ON and FINAL_RELEASE ON**, as in the commands
above. The earlier DRM-free beta is historical. Online licenses allow 90 days
since successful validation, subject to signed expiry/revocation; permanent
offline activation is a separate flow. Current dashboard: 10 activations, trials
OFF. Read `MOONBASE_INTEGRATION.md` and `FINAL_RELEASE_2.0.0.md`.

Verify both bundles contain `Contents/Resources/RCLBuildConfig.txt` with
`version=2.0.0`, `moonbase=ON`, `final=ON`, plus `MoonbaseNotices.txt`. Test browser
activation, restart, multiple instances/hosts, offline-file activation and a
non-ASCII Windows profile. Missing licenses must silence MIDI, Trigger and
audition while allowing sample/preset editing. Never log customer tokens.

PACE is developer AAX signing only; no customer iLok/PACE DRM. Re-sign and
revalidate Windows binaries; Mac validation is not Windows evidence. The Windows
installer still needs review for these final DRM artifacts and optional AAX;
the macOS installer guard does not protect Windows packaging.

For another plugin, follow the reuse section in `AAX_BUILD_AND_SIGNING.md` and
substitute that product's identities, page table, targets, and PACE configuration.
Do not reuse Lite's product IDs or Wrap GUID.

## References

- [CMake Visual Studio 17 2022 generator](https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2017%202022.html)
- [Avid's Pro Tools plug-in locations](https://kb.avid.com/pkb/articles/en_US/faq/pro-tools-plugins-folder-location)
- Installed Avid toolkit readme and installed `wraptool help` on the target
  machine are authoritative for its supported operations.
- Repository `NewProject/CMakeLists.txt`, `tests/CMakeLists.txt`, and
  `.github/workflows/validate.yml` are authoritative for build/test wiring.
