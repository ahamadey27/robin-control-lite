# Robin Control Lite 2.0.0 — Moonbase Beta 1

Built September 22, 2026 at Alex's request for a private Moonbase upload,
download, fresh-install and activation test. **This beta explicitly includes
Moonbase**, separate from the earlier DRM-free Beta 1 plan. Binary version is
still 2.0.0; “Moonbase Beta 1” identifies this test package, not a public release.

## Upload artifact

`Releases/Installers/Beta/Robin Control Lite 2.0.0 Moonbase Beta 1.pkg`

- Universal **x86_64 + arm64**, macOS **11.0+**.
- Selectable **VST3, AU, AAX Native** installation; no Standalone.
- Moonbase RCL product configured with **90-day online-license offline grace**.
- Conduit DSP Developer ID signatures; AAX **PACE-signed, not wrapped**.
- Apple notarization **Accepted**, ticket stapled and validated.
- Gatekeeper: **accepted / Notarized Developer ID**.
- Checksum alongside the installer in `.pkg.sha256`.
- **Not uploaded or published to Moonbase by this build task.** Use this package
  for the private release upload and customer download test.

All package payload files and symlinks match the signed staging bundles.
Each component is non-relocatable: installation targets the designated system
plug-in folder, not a discovered development/backup copy. The package presents
private-beta activation instructions; public release EULA/privacy review remains
on the final-release checklist.

## Build and validation

Source: commit `222a386` (Moonbase integration). Local JUCE 8.0.15,
Command Line Tools compiler, macOS 15.4 SDK, Release optimization,
`RCL_ENABLE_MOONBASE=ON`, `RCL_FINAL_RELEASE=OFF`, Standalone/copy-after-build OFF.
Build directory: `NewProject/build-moonbase-beta-2.0.0`.

| Check | Result |
|---|---|
| VST3 pluginval strictness 10, signed universal bundle | PASS |
| AU pluginval strictness 10 | PASS |
| Native arm64 auval | PASS, reports **2.0.0 / 0x20000** |
| Intel/Rosetta auval | PASS, reports **2.0.0 / 0x20000** |
| Unsigned AAX Native | All 13 applicable checks PASS |
| PACE + strict Apple verification, signed AAX | PASS |
| Signed AAX Native | All 13 applicable checks PASS |
| AAX DSP/HDX cycle-count check | N/A: Native-only plugin |
| Universal architectures and minimum OS | Both slices, 11.0, all formats |
| Installer signature, payload integrity, notarization and ticket | PASS |

The initial AU copy was not discovered by macOS. Refreshing the component
registrar resolved it. A broad `auval -a` inventory scan stalled among unrelated
third-party plug-ins and was stopped; the subsequent **RCL-only** native auval,
Rosetta auval and pluginval checks all passed. Preserve that distinction in the
logs: the inventory scan's termination is not an RCL test failure. This is the
first recorded conclusive AU **2.0.0** validation, resolving the older version
registry caveat for this particular beta.

Evidence, manifests, hashes and exact logs:
`Releases/Testing/2.0.0/moonbase-beta-1-20260922/` (local, Git-ignored).
`beta-manifest.json` identifies the finished installer and signed executables.
Apple submission: `03a890de-af4b-462b-b04f-694ef6e4a828`.

## Fresh-install state

Alex closed the hosts and ran the prepared administrator removal script.
Five installed RCL bundles were moved into `previous-installations/` under
the evidence directory, outside all DAW scan folders:

- System and user VST3 copies (1.0.1).
- System and user AU copies (1.0.1).
- System PACE-signed AAX copy (2.0.0, pre-Moonbase).

RCL VST3/AU installer receipts were forgotten; no AAX receipt existed. All six
possible system/user RCL VST3/AU/AAX install locations were verified empty at
completion. The AU was temporarily installed for validation and removed again.
Samples, presets and preferences were preserved. No RCL Moonbase license cache
existed at the final check. The premium Robin Control and differently identified
legacy `round-robin-lite` plug-ins were not removed.

**The new beta is deliberately not installed**, ready for Alex's Moonbase
download and clean-install test. A DAW may need a fresh scan after installation.

## Remaining user test

1. Upload this exact notarized `.pkg` to the private Moonbase test release.
2. Download it through the intended customer path and install desired formats.
3. Load RCL, activate an account that owns the free RCL entitlement, and verify
   playback in AU/VST3/retail Pro Tools AAX.
4. Save/reopen a session, close the editor, and verify continued playback/rendering.
5. Check deactivation/reactivation and offline activation separately.

Earlier retail Pro Tools success was for the DRM-free AAX candidate, not this
Moonbase beta. Live account activation, real host playback and customer download
tests are not implied by automated validation. Windows is outside this macOS
package; follow `MOONBASE_INTEGRATION.md` and `WINDOWS_BUILD_AND_AAX.md` there.

## Rebuilding a later beta

Use a fresh staging/evidence directory and a new beta label; never overwrite
PACE-signed staging. Follow the universal command in `MOONBASE_INTEGRATION.md`,
then the signing-only workflow in `AAX_BUILD_AND_SIGNING.md` for AAX. Sign VST3/AU
with the Conduit DSP application identity. To package a later signed candidate:

```sh
RCL_ARTEFACTS=/absolute/path/to/new/signed-staging \
RCL_INCLUDE_AAX=1 RCL_BETA_LABEL='Moonbase Beta 2' \
RCL_OUTPUT_DIR="$PWD/Releases/Installers/Beta" \
INSTALLER_SIGN='Developer ID Installer: CONDUIT DSP LLC (QS378YGT2W)' \
  bash installer/build-installer.sh
```

The script verifies AAX signatures and the page table before including AAX,
rejects stale bundle versions and existing output filenames, and preserves PACE
symlinks. Repeat validation, notarization, stapling and payload inspection for
each newly built package.
