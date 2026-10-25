# Windows 2.0.1 build record — October 25, 2026

Source: merge commit `937cae9`, combining GitHub main `83299a2` with the
existing Windows release history through `4e60990`. The clean local branch had
seven Windows commits absent from remote main; those were preserved rather than
reset. Two documentation conflicts were reconciled. No push or publication.

Read `WINDOWS_2.0.1_AGENT_HANDOFF.md` for scope and
`WINDOWS_FINAL_2.0.0.md` for the established Windows signing workflow.
The merged AAX validator supports native Windows with explicit `--dsh`.

The existing F5 correction survives: the footer logo belongs to each editor,
and keyed/output images use software storage. No artwork/layout redesign.
Remote fixes for symbol visibility, sample-pool transactions and bounded state
parsing are retained. Parameter IDs, identity and licensing policy are unchanged.

Fresh build: `NewProject/build-windows-2.0.1`, x64 Release, MSVC 19.43.34810,
Visual Studio 2022, Windows SDK 10.0.22621.0. Moonbase and final mode ON;
Standalone and automatic installation OFF; tests ON. All four targets built.
Pinned JUCE 8.0.15, AAX SDK 2.9.0 and Moonbase 4.4.0 sources were reused from
the earlier final build, with normal build-local Moonbase hardening.
All 4,425 JUCE archive files and 148 Moonbase archive files match their pinned archives; SHA-256 pins pass (`dependency-parity.json`). Additional installed AAX SDK files are outside that archive comparison.

Evidence: `Releases/Testing/2.0.1/windows-x64-20261025/` (ignored).
`configure.log`, `build.log`, `source-hashes.json` and `source.patch` preserve
build details and packaging changes beyond the merge. The installer and AAX
signing guards now require 2.0.1; stable installer AppId is preserved.

Verified so far:

- Engine/state/pool regression executable: PASS, exit 0.
- Synthetic Moonbase licensing executable: PASS, exit 0.
- Unsigned and Azure-signed VST3 strictness-10 pluginval: PASS, GUI included,
  process exit 0. Editor, editor while processing and editor automation pass.
- Both PE binaries: x64, product/file version 2.0.1.
- VST3 exports: only ExitDll, GetPluginFactory, InitDll.
- AAX exports: only the seven ACF entry points; no JUCE/Moonbase internals.
- Unsigned AAX: all 13 applicable checks PASS, Native-only descriptor confirmed; DSP/HDX cycle-count N/A.
- AAX PACE signing-only and Azure Authenticode verification: PASS, timestamped Conduit DSP LLC / Robin Control Lite, signed but not wrapped. Signed AAX validation and installer results follow below.

Manual Windows Pro Tools/PatchWork playback, activation/offline behavior,
installation/upgrade and full host-exit acceptance remain untested for these
exact artifacts. Automated GUI lifecycle checks do not replace retail host tests.
Existing installed plug-ins, samples, presets and licenses were not changed.


## Completed signing and packaging

Signed AAX validation: all 13 applicable checks PASS, runner exit 0;
DSP/HDX-only cycle counts N/A. Both unsigned/signed suites discovered the
installed test list and confirmed Lite's Native-only descriptor. Toolkit
configuration hash matches the previously verified preference-preserving setup.

Combined installer: `Releases/Installers/RobinControlLite-2.0.1-Windows.exe`.
SHA-256: `20A819CEF4284E4912E620C07A75893950B7B3369C4B6F8E7B751A8118C473CB`.
A sibling `.exe.sha256` file is included. The official copy matches the evidence
copy exactly. Both AAX and VST3 are included; no AU or Standalone.

Inno Setup compiled from the exact signed bundles after all automated validation
passed. Packaging verified identity/version/final markers, notices/page table,
PACE and Authenticode signatures before compression. Setup and uninstaller
signing/verification passed; the final setup is timestamped CONDUIT DSP LLC.
The stable AppId and existing uninstall destinations are retained.
`installer-build.log` records included payloads and signature checks;
`signed-payload-manifest.json` records every staged payload file/hash.
No extracted-installer inventory or installation/upgrade test is claimed.

Signed inner AAX SHA-256:
`F63C144D0C5C6C439DC4F26482170E1E4AAFFBDF701038B4A485C8D06ABEE2A7`.
Other exact payload hashes are in the manifest. Installation and retail Windows
Pro Tools/PatchWork, activation/offline and manual host matrix remain pending.
Close audio hosts before running the installer, then test AAX and VST3 playback,
editor close/reopen/removal, session restore and clean host exit.
Nothing was installed, uploaded or published by this task.
