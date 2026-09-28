# Windows Moonbase Beta 1 — 2026-09-23

Private x64 VST3 beta for a fresh local install. Binary version 2.0.0;
Moonbase ON, final-release flag OFF. Not installed or published.

Installer: `Releases/Installers/Beta/RobinControlLite-2.0.0-Moonbase-Windows-Beta-1.exe`

SHA-256: `b2da62645f3797c2cb417b0b78d5e8883a34d92862a05296672ae4f9b35ee5a8`

Built from merge `8d98d0f` plus the local MSVC callback fix and new beta installer
script. The UI-size menu callback now captures a locally constructed SafePointer;
this avoids MSVC's nested lambda initializer error while retaining lifetime checks.

## Build

Visual Studio 2022, MSVC 19.43.34810, CMake 4.3.2, Windows SDK 10.0.22621.0.
JUCE and Moonbase use the repository's checksum-pinned downloads and hardening.
No AAX SDK was found at the configured path; this installer includes VST3 only.

```powershell
cmake -S NewProject -B NewProject/build-windows-beta-2.0.0 -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH=C:/Users/hamad/SDKs/JUCE-8.0.15 -DJUCE_AAX_SDK_PATH=C:/Users/hamad/SDKs/aax-sdk-2-9-0 -DRCL_ENABLE_MOONBASE=ON -DRCL_FINAL_RELEASE=OFF -DRCL_BUILD_STANDALONE=OFF -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF -DRCL_BUILD_TESTS=ON
cmake --build NewProject/build-windows-beta-2.0.0 --config Release --target RobinControlLite_VST3 RobinControlLiteTests RobinControlLiteLicensingTests --parallel 4
& 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe' installer/windows/RobinControlLiteBeta.iss
```

Before packaging, verified bundle version=2.0.0, moonbase=ON, final=OFF and
MoonbaseNotices.txt. Inno Setup successfully packaged all bundle resources and
the x64 binary. The installer is unsigned and has not yet been run.

## Validation and fresh-install state

- Release build and processor/engine regression suite: passed.
- Licensing suite: all 23 checks passed, including real processor gating and
  synthetic offline activation. Live customer activation remains untested.
- pluginval strictness 10: completed tests and logged SUCCESS, including editor
  and parameter fuzzing. The validator remained running after SUCCESS and needed
  termination; this is not a verified clean process exit. Check host shutdown
  during manual testing before treating this as release-ready.
- Removed `C:\Program Files\Common Files\VST3\Robin Control Lite.vst3` using UAC.
  Verified no RCL copies in the checked system/user VST3 folders, C:\VST3 or
  C:\VstPlugins. Broader Program Files/AppData scan found only that installed
  bundle. Custom folders outside these locations were not exhaustively scanned.
- Samples, presets, and old project build artifacts were preserved.

Local logs: `cmake-windows-beta-build.log`, `engine-windows-beta.log`,
`licensing-windows-beta.log`, `pluginval-windows-beta-results.log`,
`pluginval-windows-beta-stdout.log`, and `inno-windows-beta.log`.

Run the installer, rescan VST3s in the DAW, activate RCL, and test sample loading,
MIDI/Trigger playback, session save/reopen, UI scaling, and host shutdown.

## Windows AAX compilation — 2026-09-24

The separate `NewProject/build-windows-aax-beta-2.0.0` build now successfully
produces an unsigned x64 AAX, version 2.0.0, Moonbase ON and final OFF. This does
not change the VST3-only installer above. Source is `8d98d0f` plus the existing
local PluginEditor fix. No source changes were needed for this retry.

The September 23 attempt failed with MSVC C1060 (compiler out of heap space).
The successful retry used one build worker, `/MP1`, x64 host tools, and disabled
MSBuild multiprocess compilation. The process environment also required removing
duplicate `Path`/`PATH` entries before launching MSBuild:

```powershell
$rclBuildPath = $env:PATH
[Environment]::SetEnvironmentVariable('Path', $null, 'Process')
[Environment]::SetEnvironmentVariable('PATH', $null, 'Process')
[Environment]::SetEnvironmentVariable('PATH', $rclBuildPath, 'Process')
$env:CL='/MP1'
cmake --build NewProject/build-windows-aax-beta-2.0.0 --config Release --target RobinControlLite_AAX --parallel 1 -- /p:PreferredToolArchitecture=x64 /p:MultiProcessorCompilation=false
```

This cache uses JUCE from the earlier beta build's `_deps/juce-src` and AAX SDK
2.9.0 bundled beneath `modules/juce_audio_plugin_client/AAX/SDK` there.
Log: `cmake-windows-aax-retry.log`; successful exit code 0.

Bundle: `NewProject/build-windows-aax-beta-2.0.0/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin`.
Inner `Contents/x64/Robin Control Lite.aaxplugin`: 7,361,536 bytes, PE machine
0x8664, file/product version 2.0.0, SHA-256
`10d704efe5d4ac37f6ec89b824ec8768faf6b591e1346965b2575ef661eb48f8`.
Verified build marker, Moonbase notices, and page table are present; packaged XML
hash matches source. No AAX host/validator tests or signing have run.

Windows PACE signing tools, AAX Validator/DigiShell, and Pro Tools were not found
in the locations checked. iLok License Manager is installed; user reports
inserting the developer iLok. Next: obtain Windows signing/validation tools,
validate the unsigned bundle, establish Windows signing requirements, sign a
separate staged copy, verify/revalidate, then test in retail Pro Tools.

## Windows DigiShell recovery and validation — 2026-09-24

**Result: PASS — all 13 applicable checks**, including the three DTT helper
checks, page-table parsing/automation, 1,000 load/unload cycles, parameters and
all three traversal modes. DigiShell sessions and the runner exited cleanly.
`test.cycle_counts` is **N/A**, not PASS: the installed test describes DSP cycle
counts, and the descriptor confirms Lite's two Native types with no DSP types.

Evidence: `Releases/Testing/2.0.0/aax-windows-unsigned-20260924-verified/` contains
`summary.json`, per-test logs, `dtt-logs/`, the exact `Main.valconfig` and toolkit
adjustment manifest. All 44 checked toolkit binaries/Ruby sources matched their
originals. The parser accepted the 13 final results and rejected the earlier
three failures plus synthetic command-error/aborted/wrong-ID/duplicate cases.
The Windows preference guard rejected the original config before launching DSH.

DigiShell was subsequently provided at
`C:/Users/hamad/Desktop/conduit dsp files/DigiShell/dsh.exe`, version
24.9.0x15, with AAX Validator 2024.6.0. The unsigned beta binary is unchanged:
Moonbase ON, final OFF, SHA-256
`10d704efe5d4ac37f6ec89b824ec8768faf6b591e1346965b2575ef661eb48f8`.

The apparent terminal freeze was the Windows console input interface. Avid's
bundled DTT source uses `dsh.exe -a` with each command followed by a complete YAML
document. Plain piped command lines are not that protocol. Windows AAXHost also
requires native backslash bundle paths; YAML single quotes preserve them without
escape interpretation. `scripts/test-aax-native.py` now handles this on Windows,
hashes the x64 executable and validator configuration, and retains Mac behavior.
It rejects command errors, incomplete/failed results, and wrong or duplicate IDs.
On Windows it also refuses DTT configurations without explicit preference and
DigiOptions preservation.

The original toolkit's `AAXValidatorResources/Main.valconfig` supplies literal
single quotes around Windows DTT arguments. Ruby therefore sees `'bundle_path`
instead of `bundle_path`, leaving default `Trim.aaxplugin` or
`LoadUnloadFail.aaxplugin` selected. The first full run recorded ten passes and
three failures for this tool configuration problem; it is preserved under
`Releases/Testing/2.0.0/aax-windows-unsigned-20260924-complete/`.

For the corrected run, an isolated toolkit copy lives at
`Releases/Testing/dsh/`. The original toolkit, plug-in and test scripts were not
modified. In the copy's `Main.valconfig`, the `process_dtt` arguments use
double-quote escaping for Windows and preserve existing Pro Tools preferences
and DigiOptions:

```text
arguments {
    elem: "--no_pref_delete"
    elem: "--no_move_options"
    elem: "--port=$AAXVAL_TCP_PORT$"
    elem: "-a"
    elem: "\"bundle_path=$AAXVAL_PARAM_AAXPLUGIN$\""
    elem: "-a"
    elem: "\"uniq_id=$AAXVAL_PARAM_UNIQ_ID$\""
}
```

Use these changes only in a Windows validation copy. Original config SHA-256:
`3028df0f10ac5680aa859cb3bbc78c96aa142a22670b2a9b96d0decdd6f10c1a`.
Corrected config SHA-256:
`73b38cbeb9d52ae97ac35e6473b526f79d6e5710952e0dd6935988ffb7c4c887`.
The original unadjusted DTT defaults invoke preference deletion; the corrected
copy explicitly disables that. The initial unadjusted helper logs recorded this
operation, but do not establish whether any existing preferences were present.

Run outside the restricted agent sandbox, which prevented helper logging and
execution. Choose a **new** output directory for each run:

```powershell
python scripts/test-aax-native.py 'NewProject/build-windows-aax-beta-2.0.0/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin' --dsh Releases/Testing/dsh/dsh.exe --output Releases/Testing/2.0.0/aax-windows-unsigned-20260924-verified
```

This validates the unsigned **beta** only. PACE signing, validation of the signed
copy, retail Windows Pro Tools acceptance, AAX packaging and final-build flags
remain separate checkpoints. No installer or customer licensing configuration
was changed during this validation work.
