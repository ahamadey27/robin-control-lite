# AAX build and signing handoff

Last checked: 2026-09-20. Product: **Robin Control Lite**. Release candidate: **2.0.0**.

This is the AAX implementation handoff for future agents. Read `AGENTS.md`,
`TESTING.md`, and `RELEASE_2.0.0.md` first. The historical v1.0 commands in
`release-spec.md` are not verified PACE 6 instructions. In particular, do not
copy its password-on-command-line example or assume Apple signing is irrelevant
to AAX: the installed WrapTool supports the Apple signature as part of signing.

## Current evidence

| Item | Observed state |
| --- | --- |
| AAX SDK | `/Users/alex/SDKs/aax-sdk-2-9-0` |
| Validator | `/Users/alex/SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh`; reports DigiShell v24.9.0x14 despite folder name |
| PACE tool | `/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool` |
| PACE tool version | 6.0.1 GM, build 6847, built 2026-08-25 |
| PACE installation | AAX Code Signing Tools 6; `wraptool` is not on shell PATH |
| iLok License Manager | `/Applications/iLok License Manager.app` exists |
| Apple application identity | `Developer ID Application: CONDUIT DSP LLC (QS378YGT2W)` verified outside sandbox |
| Avid/PACE authorization | Alex reports an approval email; actual signing credentials not yet verified |
| Developer tool entitlements | Alex reports “Pace Tools”, “Pace Central Access”, and “Edan Tools” on the physical iLok, plus a PACE Central UI area (2026-09-20); names/status not independently inspected |
| Signing-capable iLok / publisher configuration | Unverified; do not infer from tool installation |
| Signed AAX / retail Pro Tools acceptance | Pending |

The unsigned 2.0.0 AAX compiled for x86_64 + arm64, but the full validator run
was **not clean**. `test.page_table.load` reported two failures (“Failed to load
page tables library”), and `test.cycle_counts` ended `E_LOST` with a broken-pipe
helper error. Other reported stages passed. These need investigation before
release; do not assume either is harmless or caused by missing PACE signing.
Logs are under `Releases/Testing/2.0.0/`.

The sandboxed `security find-identity -v -p codesigning` returned zero identities;
the approved check outside the sandbox found the identity above. An empty
sandbox result is not proof of a missing certificate.

## Product identity and scope

- Product/target: `Robin Control Lite` / `RobinControlLite`.
- Manufacturer/plugin codes: `Cdsp` / `rcll`.
- Bundle and AAX identifier: `dsp.conduit.RobinControlLite`.
- AAX product ID uses the existing plugin code; do not change it for a version bump.
- AAX Native instrument, mono/stereo output, MIDI input; not AAX DSP.
- Keep APVTS parameter IDs and existing preset state compatible.
- **User requirement confirmed 2026-09-20: no customer iLok DRM.** Lite remains
  free. Use developer AAX code signing only; do not add a protection wrapper,
  runtime PACE licensing checks, or a Lite requirement for an iLok account,
  activation, USB key, or iLok Cloud session. Pro Tools' own licensing is separate.
- Standalone is a local debugging option, OFF in release builds.

Licenses for PACE developer tools on Alex's USB key do not by themselves impose
licensing on customers. They also do not establish that the key has its signing
certificate or that the correct publisher configuration is ready. Verify those
separately using the approval instructions and authenticated PACE documentation.

## Build checkpoint

`NewProject/CMakeLists.txt` is authoritative. AAX is enabled when the SDK is found.
Check the configure log and explicitly build the AAX target: the fallback when
the SDK is absent silently omits AAX from the normal all-target build.

Use the exact configure command and artifact paths in `RELEASE_2.0.0.md`.
Build both Intel and Apple Silicon slices before signing. Check every bundle's
`Info.plist` version, executable architectures, and Mach-O deployment target.
Keep unsigned build artifacts separate from signed release staging.

Compile success is not signing success. Developer tools may load an unsigned
AAX that retail Pro Tools refuses. Do not label an unsigned bundle distributable.

## Authorization checkpoint — next user-assisted step

1. Plug in the developer's iLok USB and sign into iLok License Manager.
2. Inspect the connected iLok's details and icon. Official iLok help says an iLok
   certified for digital signing has a **certificate seal** on its icon.
3. Review the Avid/PACE approval instructions for the exact publisher account,
   signing authorization, and wrap configuration or publisher identifier.
   Do not assume the entitlement must be named “AAX Commercial”; the exact
   setup must come from the actual approval and PACE instructions.
4. Open the PACE documentation linked below while signed into the authorized
   account. The automated fetch redirected to a login page on 2026-09-20.
5. Record only non-secret setup facts here after verification. Passwords,
   activation codes, private keys, and authentication tokens stay out of the
   repository, chat, shell history, and command logs. Do not export private keys.

The installed tool documents secure credential caching in the Keychain. iLok
License Manager login and WrapTool credentials are separate. Use the official
interactive setup for initial authentication; do not put a password into a
build script. If the signing service is offered, confirm account entitlement
before using it; tool support alone does not establish access.

## Signing checkpoint

Read the installed tool's current help before running a signing command:

```sh
"/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool" --version
"/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool" help
```

The 6.0.1 help establishes:

- `sign` digitally signs without applying a protection wrapper. `wrap` is a
  different operation and is outside this product's authorized scope; always
  specify `sign` explicitly rather than invoking the default operation.
- macOS signing accepts `--signid` for the Apple Developer ID Application identity.
- `--wcguid` supplies a verified wrap configuration. An alternative documented
  signing form uses explicit publisher name/number. Choose the form authorized
  for this publisher; neither value has been verified yet.
- `--dsigharden` enables the signing options needed when notarizing separately.
- `--out` permits preserving the original input; otherwise signing can be in place.
- `verify --in` verifies the PACE signature.

The following is a **template, not a verified signing invocation**. Use only
after the account, signing iLok, and actual configuration have been confirmed:

```sh
"/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool" sign \
  --account "$PACE_ACCOUNT" \
  --wcguid "$PACE_WCGUID" \
  --signid "Developer ID Application: CONDUIT DSP LLC (QS378YGT2W)" \
  --dsigharden \
  --in "$AAX_UNSIGNED_BUNDLE" \
  --out "$AAX_SIGNED_BUNDLE"
```

Use an explicit output in release staging. Never run stripping, `lipo`, another
ad-hoc signature, or a build over the signed bundle afterward. Validate the final
PACE and Apple signatures, timestamp, architectures, version, and runtime flags.
Do not independently re-sign a PACE-signed AAX to “repair” it without following
PACE's current documented procedure.

```sh
"/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool" verify --in "$AAX_SIGNED_BUNDLE"
codesign --verify --strict --verbose=2 "$AAX_SIGNED_BUNDLE"
codesign -dv --verbose=4 "$AAX_SIGNED_BUNDLE"
```

## Validation and packaging checkpoints

1. Run DigiShell against the built AAX, then again against the final signed AAX:

   ```sh
   printf '%s\n' \
     'load_dish aaxval' \
     "runtests \"$AAX_SIGNED_BUNDLE\"" \
     'exit' | /Users/alex/SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh
   ```

   Require **every** test result to pass, with no `E_COMPLETED_FAIL`, `E_LOST`,
   or failed/incomplete helper suite. A final `E_COMPLETED_PASS` can describe
   only the last test; it is not proof the overall run passed. DigiShell returned
   exit code 0 for the partial-failure run observed here.
   The older `-e validator-batch=...` command in `release-spec.md` fails with
   `command line parsing failed` in this installed 2024.6 validator. Quoting
   the path differently does not fix it. Use the stdin interface above.
2. Install the signed bundle to `/Library/Application Support/Avid/Audio/Plug-Ins/`
   for a controlled retail Pro Tools test. Close Pro Tools before replacing a
   plugin. Preserve any existing installed build before replacement.
3. In retail Pro Tools, test scan/load, mono/stereo instrument tracks, MIDI,
   sample audition, Trigger/Panic, all UI sizes, sample flashes, automation,
   preset/session save/restore, and reopening the editor. Check that a customer
   without the developer's signing iLok does not receive a Lite activation prompt.
4. Extend `installer/build-installer.sh` and `installer/distribution.xml` to
   include a verified signed AAX, with matching uninstall support. At this
   checkpoint these files still package **only VST3 and AU**.
5. Build an Apple-signed installer, submit for notarization, inspect any failures,
   staple/validate, inspect installer payloads, and test on a clean machine.
   Including an AAX after notarization requires a new package and submission.
6. Windows VST3/AAX require a separate Windows build, PACE setup, validation,
   and packaging. A universal macOS binary is not a Windows binary.

## References

- [Avid AAX developer program](https://developer.avid.com/aax/)
- [PACE AAX Code Signing Tools documentation](https://docs.paceap.com/lite/SDK/aax/overview/)
  — link shipped with the installed tools; requires authenticated access here.
- [iLok License Manager icon meanings](https://help.ilok.com/faq_ilm.html)
  — digital-signing certificate seal.
- [Steinberg AAX wrapper documentation](https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Wrappers/AAX%2BWrapper.html)
  — confirms retail Pro Tools requires PACE-signed AAX.
- Installed `wraptool help` — authoritative for locally available command flags;
  do not commit proprietary tool binaries or copy the full manual into this repo.
