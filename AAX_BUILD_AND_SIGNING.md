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
| Installer provenance | Alex confirms installing `PACECodeSigningForAAXSDKMac_v6.0.1_f802503d.zip` downloaded from PACE Central (2026-09-20), matching the installed tool version/revision |
| iLok License Manager | `/Applications/iLok License Manager.app` exists |
| Apple application identity | `Developer ID Application: CONDUIT DSP LLC (QS378YGT2W)` verified outside sandbox |
| PACE onboarding | Welcome email supplied by Alex on 2026-09-20 explicitly confirms access to the digital-signing-only Fusion SDK subset |
| Signing method | Application receipt specifies Cloud AAX Signing: No; follow the local developer iLok USB workflow |
| Developer tool entitlements | User screenshot confirms PACE Tools, PACE Central Access, and Eden Tools on the selected physical iLok; displayed expiration 2027-10-01 |
| Signing-capable iLok | Certificate seal confirmed; subsequent local signing succeeded on 2026-09-20 |
| Publisher configuration | SDK 6 / Signing Only configuration successfully used; PACE verification identifies Conduit DSP LLC and Robin Control Lite |
| Signed AAX | PACE and strict Apple verification passed outside sandbox; timestamp 2026-09-20T21:21:29Z; signed, not wrapped |
| Retail Pro Tools acceptance / notarization | Still pending |

The unsigned 2.0.0 AAX compiled for x86_64 + arm64, but the full validator run
was **not clean**. `test.page_table.load` reported two failures (“Failed to load
page tables library”), and `test.cycle_counts` ended `E_LOST` with a broken-pipe
helper error. Other reported stages passed. These need investigation before
release; do not assume either is harmless or caused by missing PACE signing.
Logs are under `Releases/Testing/2.0.0/`.

### Product/configuration supplied on 2026-09-20

- Wrap GUID: `8F95C7F0-B538-11F1-8437-00505692AD3E`.
- Product GUID: `7B367AC0-B538-11F1-B096-005056920FF7`.
- PACE Auth ID: `0x3599977b` (PACE metadata, not a replacement for JUCE's frozen plugin identity).
- Product: Robin Control Lite; status Active; Use Dev Data No.
- SDK version 6; Experience version 2; Customer Experience **Signing Only**.
- Digitally sign binary true; Encrypt binary false; no beta expiry set.
- Generic portal fields also display licensing warnings and LicenseSupport
  autoinstall settings. Do not interpret those fields alone as actual runtime
  behavior. Always use explicit `wraptool sign`, never `wrap`, and verify the
  final plugin in a customer environment for absence of PACE activation prompts.

First signing trial used this GUID, the confirmed Apple identity, `--dsigharden`,
and a separate output at `Releases/Testing/2.0.0/signing-trial/AAX/`.
The sandbox initially blocked the PACE local service. The approved unsandboxed
retry exited 2: **You must specify a password for your account.** Alex then ran
the local setup script. Retrying the same command without a password argument
succeeded using the cached credentials.

The signed candidate is:
`Releases/Testing/2.0.0/signing-trial/AAX/Robin Control Lite.aaxplugin`.
PACE `verify` returned 0, identified Conduit DSP LLC / Robin Control Lite, and
explicitly reported **The binary was signed, but not wrapped.** Publisher ID:
`0x412b8238`. Apple `codesign --verify --strict --verbose=2` returned 0 outside
the sandbox. Certificate chain ends in Apple Root CA; the Developer ID identity
and team are the expected Conduit DSP values. Timestamp: 2026-09-20T21:21:29Z;
hardened runtime enabled; version 2.0.0; x86_64 + arm64 retained. Original AAX
executable SHA-256 still matches the unsigned manifest. File hashes and recorded
verification results are in `Releases/Testing/2.0.0/aax-signing-evidence.json`.

Two diagnostic caveats from this successful run:
- Signing printed an exception about validating an existing signature but exited
  0. Subsequent independent PACE and Apple verification passed. Do not classify
  the diagnostic alone as final failure, or exit 0 alone as proof of success.
- Sandboxed Apple verification misleadingly reported an invalid arm64 signature
  and unavailable authority. The identical unmodified bundle passed outside the
  sandbox, where the full certificate chain and bound Info.plist were visible.

PACE's default v1 signature compatibility adds a symlink to the package. Preserve
symlinks when staging/packaging (e.g. `ditto`); do not use a copy mode that follows
or discards them. This candidate is not yet notarized or retail-host validated.

For a new signing machine or missing credentials, run
`python3 scripts/setup-pace-signing.py` personally in a local interactive
Terminal to authenticate and synchronize PACE's cache. The script prompts for
the account and a hidden password, then invokes the installed tool's documented
`sync` operation. PACE caches credentials in Keychain. The script stores no
password file and redacts the password from captured output; no secret is typed
into shell history. WrapTool's documented interface requires the password as a
child-process argument, so it is briefly present in process arguments. Do not
run with process tracing or share diagnostic process dumps during setup.
After successful synchronization, retry `sign` and verify the resulting bundle.

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
  iLok activation, USB key, or iLok Cloud session. Pro Tools' own licensing is separate.
- **Customer licensing provider: Moonbase DRM**, explicitly chosen by Alex on
  2026-09-20 for free Lite and all future products. The no-DRM restriction above
  is specific to PACE/iLok; it is not a prohibition on Moonbase integration.
  Moonbase is required for final v2.0.0 and explicitly excluded from Beta 1
  (the user's “beta v1”). Activation policies remain unspecified. Beta 1 still
  requires PACE signing for retail Pro Tools; omitting Moonbase does not waive it.
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

Alex supplied the PACE welcome email and application receipt on 2026-09-20.
The welcome email establishes signing-only SDK access; the application receipt
alone is not approval evidence. Neither email provides a publisher number,
configuration GUID, or confirmation of a working signing certificate. The
agreement contains both commercial and evaluation terms, so do not infer the
active distribution entitlement just from those generic clauses. Confirm the
active entitlement in the authorized account before public distribution.
Do not copy the full email, agreement, personal contact details, or account
identifier into repository documentation.

PACE's supplied onboarding route is iLok License Manager → **PACE CENTRAL**
button → **DEVELOPER > SDK Download** → the macOS `PACECodeSigningForAAXSDK`.
The portal must be launched through that button, not a guessed direct URL.
The developer reports that the button and key licenses are already present,
and the tools are installed; do not repeat installation unnecessarily.

On 2026-09-20 Alex confirmed completing all welcome-email steps, but could not
find signing setup details in the portal or access the emailed documentation.
The SDK's own documentation shortcut also points to an online page unavailable
through the automated browser. `wraptool list` succeeded outside the sandbox
but printed no cached configurations. This does not query or prove the absence
of server-side account entitlements. Its sandbox failure (`connect(): Operation
not permitted`) was a local-service access restriction, not a licensing error.

Further online research on 2026-09-20 corrected the initial recommendation to
contact support immediately:

- The welcome-email documentation URL responds successfully to ordinary HTTP
  requests and redirects to `/login?redirect=...`. Its JavaScript login component
  invokes `https://docs.paceap.com/login/initiate-oauth`, which reaches a rendered
  **PACE Antipiracy — DOCUMENTATION** sign-in form (User ID and Password).
  The account-recovery link points to iLok.com. This establishes a working login
  route, not that this account has successfully authenticated or that the final
  article is accessible. The web research tool's error did not establish a dead
  link. Have the user sign in privately; never collect the password.
- Auburn Sounds' own Dplug AAX guide describes creating product and wrap
  configurations in PACE Central. A developer's first-hand JUCE forum report
  explicitly describes a **Signing Only** configuration. These explain a route
  to obtaining a configuration GUID without waiting for it in an email. They
  are older guidance, not verification of the current PACE 6 portal layout.
  A signing-only configuration does not authorize PACE customer DRM or `wrap`.
- A developer's first-hand account of March 2026 setup reports that synchronizing
  the physical iLok after license activation downloaded the signing certificate.
  Treat this as a troubleshooting lead, not a guarantee for Alex's account.
  Official iLok help documents right-click → **Synchronize** and a certificate
  seal icon for keys certified for digital signing. Both synchronization and
  the seal are now confirmed below. Do not confuse this action in iLok
  License Manager with the separate `wraptool sync` cache command.

Alex confirmed synchronizing the physical iLok on 2026-09-20. A subsequent
`wraptool list` completed successfully with no output, as before. This lists
cached signing configurations, not certificates on the USB key, so it cannot
establish whether certificate provisioning changed. The subsequent close-up
screenshot (16:59:45) clearly shows the certificate seal above the device icon;
the earlier full-window screenshot (16:59:11) does not. This confirms the
digital-signing certification indicator after synchronization. Actual signing
was subsequently verified as described above. Do not repeat synchronization
or ask for the seal again unless a new error warrants it. Do not record the
device serial or copy the screenshots into the repository.

The portal product/signing configuration and iLok certificate-seal checks are
now done, as are local authentication and the first verified signing operation.
Next: resolve validator issues and test the signed candidate in retail Pro Tools.
No evidence currently establishes that another onboarding email is required.
Only if these steps fail should support be asked about documentation access or
certificate/configuration provisioning. Do not repeat completed installation
steps or assume a publisher number appears in a particular menu. No support
message has been sent by the agent.

1. Plug in the developer's iLok USB and sign into iLok License Manager.
2. Inspect the connected iLok's details and icon. Official iLok help says an iLok
   certified for digital signing has a **certificate seal** on its icon.
3. Use PACE Central and its authenticated signing documentation to establish
   the exact signing authorization and configuration or publisher identifier;
   these were not included in the supplied emails.
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
build script. The supplied application specifies no Cloud AAX Signing; do not
enable the cloud signing service or assume access from tool support alone.

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
  signing form uses explicit publisher name/number. This project now has the
  user-supplied GUID above; a separate customer number is not required for that form.
- `--dsigharden` enables the signing options needed when notarizing separately.
- `--out` permits preserving the original input; otherwise signing can be in place.
- `verify --in` verifies the PACE signature.

The following invocation successfully signed the separate candidate after local
authentication. Use the GUID above as `PACE_WCGUID` and preserve unsigned input:

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

- [Auburn Sounds Dplug AAX guide, code signing](https://github.com/AuburnSounds/Dplug/wiki/Dplug-AAX-Guide#step-7-code-signing)
  — framework maintainers' instructions for product/wrap configuration setup.
- [First-hand signing-only setup report on the JUCE forum](https://forum.juce.com/t/problem-signing-my-aax-plugin/46177)
  — 2021; evidence of the signing-only configuration route, not current UI labels.
- [Developer's March 2026 AAX setup account](https://note.com/kawato3/n/ne11473420ad5?hl=en)
  — first-hand report of synchronization downloading the signing certificate.
- [Official iLok USB synchronization guidance](https://help.ilok.com/faq_ilok.html)
  — sign in, right-click the physical iLok, synchronize.
- [PACE's public AAX signing onboarding overview](https://paceap.com/getting-started-with-aax-code-signing-for-pro-tools-plugins/)
  — PACE provides onboarding support following Avid approval; local signing uses
  an iLok USB to hold the signing certificate. Signing and customer licensing
  are distinct.
- [Digital Signature Protection (including AAX)](https://docs.paceap.com/fusion-protection/getting-started/aax-dsig/)
  — current onboarding link from the user-supplied PACE welcome email; automated
  browser access failed on 2026-09-20. Read via the authorized account.
- [Avid AAX developer program](https://developer.avid.com/aax/)
- [PACE AAX Code Signing Tools documentation](https://docs.paceap.com/lite/SDK/aax/overview/)
  — link shipped with the installed tools; requires authenticated access here.
- [iLok License Manager icon meanings](https://help.ilok.com/faq_ilm.html)
  — digital-signing certificate seal.
- [Steinberg AAX wrapper documentation](https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Wrappers/AAX%2BWrapper.html)
  — confirms retail Pro Tools requires PACE-signed AAX.
- Installed `wraptool help` — authoritative for locally available command flags;
  do not commit proprietary tool binaries or copy the full manual into this repo.
