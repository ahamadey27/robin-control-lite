# release-spec.md — Robin Control Lite v1.0 Ship Playbook

> **Purpose.** Sequential, command-level checklist to take the current build to a signed, notarized, distributable v1.0 across **AU + VST3 + AAX** on **Mac + Windows** (no Standalone). The broader `spec.md` covers identity decisions, format rationale, host matrix, and migration history; this file is execution-only.
>
> **Companion docs.** `spec.md` (decisions/reference), `release-prep-checklist.md` (pre-ship technical state), `spec-paperwork.md` (admin/legal). When this file disagrees with anything older, this file wins for v1.0 ship.
>
> **Legend.** 🔴 **YOU:** = step that requires you (GUI clicks, browser, physical hardware, external account, separate machine, or comms with another party). Unmarked = Claude can execute on your Mac once prerequisites are in place.

---

## 0. Inputs (frozen 2026-05-02)

### 0.1 Plugin identity
| Field | Value |
|---|---|
| Product name | `Robin Control Lite` |
| CMake target | `RobinControlLite` |
| Company | `conduit.dsp` |
| Plugin code | `rcll` |
| Manufacturer code | `Cdsp` |
| Bundle ID | `dsp.conduit.RobinControlLite` |
| Version | `1.0.0` |

### 0.2 Apple Developer
| Field | Value |
|---|---|
| Legal entity | CONDUIT DSP LLC |
| Team ID | `QS378YGT2W` |
| Apple ID | `hamadey@gmail.com` |
| Account role | Account Holder, Admin |
| Developer ID (UUID) | `76a9f962-f471-4f53-a850-7f3a3a05afeb` |

### 0.3 AAX (Avid + iLok)
| Field | Value |
|---|---|
| Avid Developer account | `hamadey@gmail.com` (active) |
| AAX SDK | `~/SDKs/aax-sdk-2-9-0` (moved from Desktop 2026-05-02) |
| AAX Validator (DigiShell) | `~/SDKs/aax-validator-dsh-2024-6-0` (moved from Desktop 2026-05-02) |
| Validator entry point | `~/SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh` |
| Pro Tools Developer | Installed |
| iLok account | `alex.hamadey` |
| iLok hardware | 2nd-gen USB key (in hand) |
| Avid commercial license | **Not yet requested** — see §4.6 |

### 0.4 Scope for v1.0
- **Mac (universal x86_64 + arm64):** AU, VST3, AAX
- **Windows (x64):** VST3, AAX
- **No Standalone, no Linux, no LV2, no CLAP** in v1.0
- **Mac signing/notarization:** mandatory
- **Windows code signing:** v1.0 ships **unsigned with SmartScreen workaround** documented in README (see §8.5). EV/OV cert is a v1.0.x improvement.

---

## 1. Sequencing decision tree

AAX commercial signing requires a request to Avid (`audiosdk@avid.com`) → Avid provisions PACE Wraptool + deposits a commercial license to your iLok account. **Lead time is variable (days to a couple of weeks)**; you can't predict it.

Pick a path on the day Mac + Win VST3/AU builds are validated and stapled:

**Path A — One-shot ship (preferred if Avid grants in time).** Tag `v1.0.0` only after all three formats on both platforms are signed (AAX = PACE, VST3/AU = Apple). Hold the release until all artifacts exist.

**Path B — Staged ship.** Tag `v1.0.0` with VST3 (Mac+Win) + AU (Mac), notarized + Mac-signed, no AAX. Ship publicly. When Avid grants the commercial license, build the AAX, PACE-sign it, tag `v1.0.1` adding AAX-only artifacts. README's install section gets an "AAX coming soon" line at v1.0 and the line is removed at v1.0.1.

**Trigger to commit to Path B:** if the date you're otherwise ready to ship is more than 7 days past Avid's first acknowledgment email and Wraptool/license still hasn't appeared, ship Path B and treat AAX as a fast-follow.

- [ ] 🔴 **YOU:** Send the Avid commercial AAX license request **today** (template in §4.6) — longest-pole item, costs nothing to start

---

## 2. Build configuration changes

- [x] CMakeLists `FORMATS` line dropped Standalone, added AAX (gated on SDK presence) — **done 2026-05-02**
- [x] AAX SDK gate detects `~/SDKs/aax-sdk-2-9-0/Interfaces/AAX.h` → enables AAX format; absent → builds VST3/AU only — **done 2026-05-02**
- [x] Standalone `/Applications` post-build copy hook removed from CMakeLists — **done 2026-05-02**
- [x] `.vscode/launch.json` switched to attach-to-process pattern (Standalone gone) — **done 2026-05-02**
- [x] CLAUDE.md "Build" section updated (drop Standalone bullet, document attach-debug pattern) — **done 2026-05-02**
- [x] **Edit `installer/build-installer.sh`** to drop the Standalone component pkg — **done 2026-05-05**
- [x] **Edit `installer/distribution.xml`** to remove the Standalone choice line — **done 2026-05-05**
- [x] (Bonus) `installer/uninstall.sh` Standalone refs cleaned (echo line, `/Applications` rm, `.standalone` pkgutil --forget) — **done 2026-05-05**
- [x] Smoke build confirmed: `Releases/Installers/Robin Control Lite 1.0.0.pkg` (9.0M) expands cleanly to `RobinControlLite-VST3.pkg` + `RobinControlLite-AU.pkg`, no Standalone — **done 2026-05-05**
  ```bash
  cd /Users/alex/Documents/Github/robin-control-redesign
  ./installer/build-installer.sh
  pkgutil --expand "Releases/Installers/Robin Control Lite 1.0.0.pkg" /tmp/rcl-pkg-check
  ls /tmp/rcl-pkg-check
  rm -rf /tmp/rcl-pkg-check
  ```

---

## 3. Apple Developer setup (Mac signing prerequisites)

One-time account/keychain operations. Don't repeat per release.

### 3.1 Generate Developer ID Application certificate
Signs the `.vst3` / `.component` bundles.

- [x] 🔴 **YOU:** Open **Xcode** → **Settings** (⌘,) → **Accounts**
- [x] 🔴 **YOU:** Confirm `hamadey@gmail.com` is signed in; CONDUIT DSP LLC team appears
- [x] 🔴 **YOU:** Select team → **Manage Certificates…** → **+** → **Developer ID Application**
- [x] Verify the cert landed in your keychain:
  ```bash
  security find-identity -v -p codesigning
  ```
  Expect a line like `1) <hash> "Developer ID Application: Conduit DSP LLC (QS378YGT2W)"`
- [x] 🔴 **YOU:** Copy the **exact** common-name string somewhere you'll remember — every later `codesign` invocation pastes it

### 3.2 Generate Developer ID Installer certificate
Signs the `.pkg`.

- [x] 🔴 **YOU:** Same Xcode → Accounts → Manage Certificates pane → **+** → **Developer ID Installer**
- [x] Verify:
  ```bash
  security find-identity -v
  ```
  Expect `"Developer ID Installer: Conduit DSP LLC (QS378YGT2W)"`

### 3.3 Create app-specific password for notarytool
- [x] 🔴 **YOU:** Open https://appleid.apple.com → sign in as `hamadey@gmail.com`
- [x] 🔴 **YOU:** Sidebar → **Sign-In and Security** → **App-Specific Passwords** → **+** → label `notarytool-robin-control-lite` → **Create** (jcxr-mepf-cxfn-ywhk)
- [x] 🔴 **YOU:** Copy the one-time password (looks like `abcd-efgh-ijkl-mnop`) — it won't be shown again
- [x] 🔴 **YOU:** Hand the password to Claude (or run yourself) so it can be stored in keychain:
  ```bash
  xcrun notarytool store-credentials AC_PASSWORD \
    --apple-id "hamadey@gmail.com" \
    --team-id "QS378YGT2W" \
    --password "abcd-efgh-ijkl-mnop"
  ```
- [x] Verify credentials work:
  ```bash
  xcrun notarytool history --keychain-profile AC_PASSWORD
  ```
  Empty history is fine; the call succeeding proves auth works.

### 3.4 Back up certificates
- [x] 🔴 **YOU:** Keychain Access → **login** keychain → **My Certificates**
- [x] 🔴 **YOU:** For each `Developer ID …: Conduit DSP LLC` entry: right-click → **Export** → `.p12` format → strong password
- [x] 🔴 **YOU:** Move both `.p12` files + their passwords + the app-specific password to your password manager

---

## 4. AAX setup (one-time)

### 4.1 SDK paths
- [x] `~/SDKs/aax-sdk-2-9-0/` — SDK headers/libs JUCE links against — **moved 2026-05-02**
- [x] `~/SDKs/aax-validator-dsh-2024-6-0/` — DigiShell validator — **moved 2026-05-02**
- [ ] (Optional) Symlink the validator binary onto your `PATH`:
  ```bash
  ln -s ~/SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh /usr/local/bin/dsh
  ```

### 4.2 First AAX build (eval/unsigned)
- [x] Eval AAX builds clean as part of universal Release — **verified 2026-05-02**:
  ```
  build/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin
  ```
  Eval-signed only — loads in Pro Tools Developer, rejected by retail Pro Tools until PACE wraps it (§7).

### 4.3 Validate the .aaxplugin
- [x] Run the AAX validator:
  ```bash
  ~/SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh \
    -e validator-batch="/Users/alex/Documents/Github/robin-control-redesign/NewProject/build/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin"
  ```
  Pass = clean exit. Fix any conformance errors before submitting to Avid.

### 4.4 Smoke test in Pro Tools Developer
- [ ] 🔴 **YOU:** Copy the eval `.aaxplugin` to `/Library/Application Support/Avid/Audio/Plug-Ins/`
- [ ] 🔴 **YOU:** Launch Pro Tools Developer (Dev build, NOT retail)
- [ ] 🔴 **YOU:** New session → instrument track → insert "Robin Control Lite" → load samples → trigger MIDI
- [ ] 🔴 **YOU:** Confirm: audio plays, no crash, parameter automation reaches the plugin

### 4.5 iLok preparation
- [x] 🔴 **YOU:** Download iLok License Manager from https://www.ilok.com/#!license-manager
- [x] 🔴 **YOU:** Sign in with `alex.hamadey` account
- [x] 🔴 **YOU:** Plug in the 2nd-gen iLok USB → confirm it appears in the License Manager sidebar
- [x] Verify the iLok is visible to the OS (with USB plugged in):
  ```bash
  ioreg -p IOUSB | grep -i ilok
  ```

### 4.6 Request Avid commercial AAX license (do this on day 1 of release work)
- [ ] 🔴 **YOU:** Send this email today — longest-pole item:
  - **To:** `audiosdk@avid.com`
  - **Subject:** Commercial AAX license request — Robin Control Lite (free plugin)
  - **Body:**
    ```
    Hello,

    I'd like to request a commercial AAX license and PACE Wraptool access for distribution of my free plugin.

    Developer / Avid account: hamadey@gmail.com (Conduit DSP LLC)
    Plugin: Robin Control Lite (free)
    Plugin manufacturer code: Cdsp
    Plugin product code: rcll
    Bundle ID: dsp.conduit.RobinControlLite
    Platforms: macOS (universal), Windows x64
    iLok account: alex.hamadey
    Distribution model: free download from conduitdsp.com (no charge)

    The plugin has been built against AAX SDK 2.9.0, validated with the DigiShell AAX
    Validator (clean), and tested in Pro Tools Developer. I'd like to take it through
    PACE wrapping for retail Pro Tools distribution.

    Please advise on next steps.

    Thanks,
    Alex Hamadey
    Conduit DSP LLC
    ```
- [ ] 🔴 **YOU:** Forward Avid's reply to your records when it arrives

---

## 5. Windows build environment

All steps in this section are on the Windows laptop.

### 5.1 One-time setup
- [ ] 🔴 **YOU:** Install **Visual Studio 2022 Community** (free) → during setup tick **Desktop development with C++** workload
- [ ] 🔴 **YOU:** Install **Git for Windows**
- [ ] 🔴 **YOU:** Install **CMake** 3.22+ (cmake.org or `winget install Kitware.CMake`)
- [ ] 🔴 **YOU:** Clone the repo:
  ```powershell
  cd $HOME\Documents\Github
  git clone https://github.com/ahamadey27/robin-control-lite.git robin-control-redesign
  ```
- [ ] 🔴 **YOU:** First CMake configure auto-resolves JUCE 8.0.4 via FetchContent (no local checkout needed on Windows)

### 5.2 AAX SDK on Windows
- [ ] 🔴 **YOU:** Sign in to https://my.avid.com → **My Toolkits and Downloads** → **AAX SDK 2.9.0** → download the **Windows** variant
- [ ] 🔴 **YOU:** Unpack to `C:\SDKs\aax-sdk-2-9-0\`
- [ ] 🔴 **YOU:** Note: CMake gate looks at `%USERPROFILE%\SDKs\aax-sdk-2-9-0`. Either move there, or pass `cmake -DJUCE_AAX_SDK_PATH=C:/SDKs/aax-sdk-2-9-0`

### 5.3 First Windows build
- [ ] 🔴 **YOU:** Run:
  ```powershell
  cd $HOME\Documents\Github\robin-control-redesign\NewProject
  cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_AAX_SDK_PATH="C:/SDKs/aax-sdk-2-9-0"
  cmake --build build --config Release
  ```
  Outputs:
  - `build\RobinControlLite_artefacts\Release\VST3\Robin Control Lite.vst3`
  - `build\RobinControlLite_artefacts\Release\AAX\Robin Control Lite.aaxplugin` (eval-signed)

### 5.4 Smoke install paths (Windows, manual copy for testing)
- VST3: `C:\Program Files\Common Files\VST3\`
- AAX: `C:\Program Files\Common Files\Avid\Audio\Plug-Ins\`

(The §8 installer handles this for distribution.)

---

## 6. Mac release build — signed and notarized

Run in order. Substitute the cert common-name strings from §3.1/3.2 if they differ.

### 6.1 Clean Release build (universal)
- [ ] Run:
  ```bash
  cd /Users/alex/Documents/Github/robin-control-redesign/NewProject
  rm -rf build-release
  cmake -B build-release -G Xcode \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
  cmake --build build-release --config Release
  ```
- [ ] Verify universal:
  ```bash
  file "build-release/RobinControlLite_artefacts/Release/VST3/Robin Control Lite.vst3/Contents/MacOS/Robin Control Lite"
  # expect: Mach-O universal binary with 2 architectures: x86_64, arm64
  ```

### 6.2 Sign each plugin bundle (Apple)
- [ ] Run (after §3.1 cert exists):
  ```bash
  cd /Users/alex/Documents/Github/robin-control-redesign/NewProject/build-release/RobinControlLite_artefacts/Release

  CERT="Developer ID Application: Conduit DSP LLC (QS378YGT2W)"

  codesign --force --deep --options runtime --timestamp --sign "$CERT" "VST3/Robin Control Lite.vst3"
  codesign --force --deep --options runtime --timestamp --sign "$CERT" "AU/Robin Control Lite.component"
  ```
- [ ] Verify each:
  ```bash
  codesign --verify --deep --strict --verbose=2 "VST3/Robin Control Lite.vst3"
  codesign --verify --deep --strict --verbose=2 "AU/Robin Control Lite.component"
  ```
  Clean verify ends with `valid on disk` and `satisfies its Designated Requirement`.

(AAX gets PACE-wrapped instead — see §7. Don't apply Apple `codesign` to the AAX bundle.)

### 6.3 Build the .pkg installer
- [ ] Build (after §2 installer-script edits):
  ```bash
  cd /Users/alex/Documents/Github/robin-control-redesign
  INSTALLER_SIGN="Developer ID Installer: Conduit DSP LLC (QS378YGT2W)" \
    ./installer/build-installer.sh
  ```
  Output: `Releases/Installers/Robin Control Lite 1.0.0.pkg`
- [ ] Verify the installer is signed:
  ```bash
  pkgutil --check-signature "Releases/Installers/Robin Control Lite 1.0.0.pkg"
  ```
  Should show `signed by a developer certificate issued by Apple for distribution`.

### 6.4 Notarize
- [ ] Submit (after §3.3 keychain profile exists):
  ```bash
  cd /Users/alex/Documents/Github/robin-control-redesign
  xcrun notarytool submit "Releases/Installers/Robin Control Lite 1.0.0.pkg" \
    --keychain-profile AC_PASSWORD \
    --wait
  ```
  `--wait` blocks until Apple returns a verdict (typically 1–10 min). Look for `status: Accepted`.
- [ ] If `status: Invalid`, fetch the log to diagnose:
  ```bash
  xcrun notarytool log <submission-id> --keychain-profile AC_PASSWORD
  ```
  Common causes: missing `--options runtime`, missing `--timestamp`, embedded binary not signed.

### 6.5 Staple
- [ ] Attach the notarization ticket to the `.pkg`:
  ```bash
  xcrun stapler staple "Releases/Installers/Robin Control Lite 1.0.0.pkg"
  xcrun stapler validate "Releases/Installers/Robin Control Lite 1.0.0.pkg"
  # expect: The validate action worked!
  ```

### 6.6 Gatekeeper test (clean Mac)
- [ ] 🔴 **YOU:** On a Mac that has never had Robin Control Lite installed, download the stapled `.pkg` via Safari from a fresh URL (mimics a real user)
- [ ] 🔴 **YOU:** Double-click to launch the installer — there should be **no** "developer cannot be verified" dialog
- [ ] 🔴 **YOU:** Complete install
- [ ] 🔴 **YOU:** Open Logic → AU Manager → confirm "Robin Control Lite" by "conduit.dsp" passes validation
- [ ] 🔴 **YOU:** Open Live → confirm VST3 and AU both appear

Failure here means notarization didn't staple correctly or Gatekeeper sees a quarantined embedded binary. Rerun §6.2 → 6.5.

---

## 7. AAX commercial signing (PACE Wraptool)

Depends on Avid having granted the commercial license (§4.6). If they haven't, fall back to Path B in §1.

### 7.1 Install Wraptool
- [ ] 🔴 **YOU:** Use the download link from Avid's reply to install Wraptool on **both** Mac and Windows machines

### 7.2 Confirm iLok license deposit
- [ ] 🔴 **YOU:** Open iLok License Manager → sign in as `alex.hamadey`
- [ ] 🔴 **YOU:** Confirm new "PACE Anti-Piracy" / "AAX Commercial" license appears
- [ ] 🔴 **YOU:** Drag the license onto your physical iLok USB

### 7.3 Sign the Mac .aaxplugin
- [ ] 🔴 **YOU:** Plug in the iLok USB
- [ ] Run (substituting your iLok password and the wcguid Avid sent):
  ```bash
  wraptool sign \
    --account alex.hamadey \
    --password "<ilok-account-password>" \
    --wcguid "<wcguid-from-avid-email>" \
    --in "/Users/alex/Documents/Github/robin-control-redesign/NewProject/build-release/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin" \
    --out "/Users/alex/Documents/Github/robin-control-redesign/NewProject/build-release/RobinControlLite_artefacts/Release/AAX/Robin Control Lite.aaxplugin"
  ```
- [ ] Verify:
  ```bash
  wraptool verify --in "/path/to/Robin Control Lite.aaxplugin"
  ```

### 7.4 Sign the Windows .aaxplugin
- [ ] 🔴 **YOU:** Same procedure on the Windows laptop, run from PowerShell. Wraptool's Windows binary takes the same flags. iLok USB plugged in.

### 7.5 Re-bundle into installers
- [ ] Mac: rebuild the `.pkg` (§6.3) so it picks up the now-signed `.aaxplugin`. Re-notarize and staple (§6.4–6.5) — the `.aaxplugin` is a new payload, the `.pkg` signature changes, notarization must redo.
- [ ] 🔴 **YOU:** Windows: rebuild the Windows installer (§8.4) including the signed `.aaxplugin`

### 7.6 Final AAX validation
- [ ] Re-run validator on the **signed** plugin:
  ```bash
  ~/SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh \
    -e validator-batch="/path/to/signed/Robin Control Lite.aaxplugin"
  ```
- [ ] 🔴 **YOU:** Load the **signed** plugin in **retail Pro Tools** (not the Dev build) — it should validate and load. Proof PACE signing worked.

---

## 8. Windows release build

### 8.1 Build VST3 + AAX Release
- [ ] 🔴 **YOU:** On the Windows laptop:
  ```powershell
  cd $HOME\Documents\Github\robin-control-redesign\NewProject
  Remove-Item -Recurse -Force build-release -ErrorAction SilentlyContinue
  cmake -B build-release -G "Visual Studio 17 2022" -A x64 -DJUCE_AAX_SDK_PATH="C:/SDKs/aax-sdk-2-9-0"
  cmake --build build-release --config Release
  ```

### 8.2 Smoke test (unsigned)
- [ ] 🔴 **YOU:** Reaper-Win: load VST3 → MIDI trigger → save project → reopen → state recall
- [ ] 🔴 **YOU:** Pro Tools-Win: load PACE-signed AAX (after §7.4)
- [ ] 🔴 **YOU:** FL Studio: VST3 scan, load, automation lane test (FL is the quirky one for automation)

### 8.3 Code signing decision
v1.0 ships **unsigned** (per §0.4). Document the SmartScreen workaround in README (§8.5).

If you decide to buy a Windows code-signing certificate later:
- EV cert (~$300–500/yr, Sectigo / SSL.com) — instant SmartScreen reputation, requires hardware token shipped to you
- OV cert (~$200–300/yr) — accumulates reputation over weeks/months
- Sign with `signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 /a "Robin Control Lite.vst3"`

### 8.4 Installer (Inno Setup)
- [ ] 🔴 **YOU:** Install Inno Setup 6 from https://jrsoftware.org/isinfo.php (free, scriptable)
- [ ] 🔴 **YOU:** Create `installer/windows/RobinControlLite.iss`:
  ```pascal
  [Setup]
  AppName=Robin Control Lite
  AppVersion=1.0.0
  AppPublisher=Conduit DSP LLC
  AppPublisherURL=https://conduitdsp.com
  DefaultDirName={autopf}\Common Files\VST3
  DisableDirPage=yes
  DisableProgramGroupPage=yes
  OutputDir=..\..\Releases\Installers
  OutputBaseFilename=RobinControlLite-1.0.0-Windows
  PrivilegesRequired=admin
  UninstallDisplayName=Robin Control Lite

  [Files]
  Source: "..\..\NewProject\build-release\RobinControlLite_artefacts\Release\VST3\Robin Control Lite.vst3\*"; \
    DestDir: "{commoncf}\VST3\Robin Control Lite.vst3"; Flags: recursesubdirs createallsubdirs
  Source: "..\..\NewProject\build-release\RobinControlLite_artefacts\Release\AAX\Robin Control Lite.aaxplugin\*"; \
    DestDir: "{commoncf}\Avid\Audio\Plug-Ins\Robin Control Lite.aaxplugin"; Flags: recursesubdirs createallsubdirs

  [Code]
  // Show EULA at install time — set LicenseFile=..\..\EULA.md (or convert to .rtf)
  ```
- [ ] 🔴 **YOU:** Build with `iscc installer/windows/RobinControlLite.iss`. Output lands in `Releases/Installers/RobinControlLite-1.0.0-Windows.exe`

### 8.5 SmartScreen workaround in README
- [ ] Add to README install section:
  > **Windows users:** the installer is currently unsigned. Windows SmartScreen may show "Windows protected your PC". Click **More info** → **Run anyway** to proceed. We're working on adding a code-signing certificate to remove this warning.

---

## 9. Final validation gate

Don't tag `v1.0.0` until all of these are green. Cross-references go to `spec.md` §7.

### 9.1 Automated (Mac)
- [ ] `pluginval --strictness-level 10 --validate-in-process --timeout-ms 600000 "build-release/RobinControlLite_artefacts/Release/VST3/Robin Control Lite.vst3"` → PASS
- [ ] Same for AU → PASS
- [ ] `auval -v aumu rcll Cdsp` → `AU VALIDATION SUCCEEDED.`
- [ ] `dsh -e validator-batch=...` on the (PACE-signed) `.aaxplugin` → PASS

### 9.2 Automated (Windows)
- [ ] 🔴 **YOU:** `pluginval.exe --strictness-level 10 ...` on Windows VST3 → PASS

### 9.3 Per-host smoke (spec.md §7.4 + §7.5 + §7.6)
- [ ] 🔴 **YOU:** Logic Pro (AU)
- [ ] 🔴 **YOU:** Ableton Live 12 (VST3 + AU on Mac, VST3 on Win)
- [ ] 🔴 **YOU:** Reaper 7 (VST3 + AU on Mac, VST3 on Win)
- [ ] 🔴 **YOU:** FL Studio (VST3 on Win)
- [ ] 🔴 **YOU:** JUCE AudioPluginHost (Mac + Win)
- [ ] 🔴 **YOU:** Pro Tools retail (Mac + Win, AAX) — only after PACE signing

### 9.4 Stress / perf / platform
- [ ] 🔴 **YOU:** `release-prep-checklist.md` §5 stress checklist clean
- [ ] 🔴 **YOU:** `release-prep-checklist.md` §6 perf targets met
- [ ] Compiler warnings cleaned (`release-prep-checklist.md` §11)

### 9.5 Distribution sanity
- [ ] 🔴 **YOU:** Mac: Gatekeeper test on a clean Mac (§6.6) clean
- [ ] 🔴 **YOU:** Windows: install on a fresh Windows VM works
- [ ] 🔴 **YOU:** Uninstaller (`installer/uninstall.sh` Mac, Inno-generated Windows) removes all files

### 9.6 Sanity
- [ ] About dialog version matches the git tag
- [ ] No `DBG()` / debug logs in Release
- [ ] LICENSE + EULA + Privacy.md present
- [ ] 🔴 **YOU:** README updated for end-users; SmartScreen note included; AAX-coming-soon note iff Path B

---

## 10. Distribution

### 10.1 Tag and push the release
- [ ] 🔴 **YOU:** Tag and push (visible to public — confirm before running):
  ```bash
  cd /Users/alex/Documents/Github/robin-control-redesign
  git tag -a v1.0.0 -m "Robin Control Lite 1.0.0"
  git push origin v1.0.0
  ```

### 10.2 GitHub Release
- [ ] 🔴 **YOU:** Repo → Releases → **Draft a new release**
- [ ] 🔴 **YOU:** Tag: `v1.0.0`; Title: `Robin Control Lite 1.0.0`
- [ ] 🔴 **YOU:** Body: feature summary, install instructions per platform, link to EULA + Privacy
- [ ] 🔴 **YOU:** Attach: Mac `.pkg`, Windows `.exe` installer
- [ ] 🔴 **YOU:** Publish

### 10.3 conduitdsp.com download page
- [ ] 🔴 **YOU:** Update download links to point at the GitHub Release artifacts (or self-host)
- [ ] 🔴 **YOU:** Confirm the EULA link resolves at `https://conduitdsp.com/eula/robin-control-lite/`
- [ ] 🔴 **YOU:** Confirm the privacy policy link resolves at `https://conduitdsp.com/privacy-policy/`
- [ ] 🔴 **YOU:** MailerLite email-capture form active before download starts

### 10.4 KVR Audio listing
- [ ] 🔴 **YOU:** Submit at https://www.kvraudio.com/get-listed (free, ~24h moderation)
- [ ] 🔴 **YOU:** Required fields: name, manufacturer, formats, OSes, screenshots (3–5), short + long description, version, price (free), download URL, support URL

### 10.5 Announce
- [ ] 🔴 **YOU:** Order: GitHub Release → conduitdsp.com page live → KVR submitted. Then post to your channels. Don't announce before the download URL resolves.

---

## 11. Post-ship

### 11.1 Backups
- [ ] 🔴 **YOU:** `.p12` exports of both Developer ID certs in password manager
- [ ] 🔴 **YOU:** App-specific notarytool password backed up
- [ ] 🔴 **YOU:** iLok account credentials in password manager
- [ ] 🔴 **YOU:** Wraptool installation backed up (re-downloadable from Avid, but record the download URL)

### 11.2 Calendar reminders
- [ ] 🔴 **YOU:** Apple Developer Program renewal — 30 days before expiry (lapse breaks future notarization)
- [ ] 🔴 **YOU:** JUCE Personal license re-check — every JUCE major version bump
- [ ] 🔴 **YOU:** Trademark status re-check — every 12 months

### 11.3 Open issues for v1.0.x
- [ ] Compiler warning cleanup pass (`release-prep-checklist.md` §11)
- [ ] AU "Current program is -1" (factory presets) — accepted for v1.0, revisit
- [ ] Windows code-signing certificate purchase decision
- [ ] AAX shipping (if Path B) → tag v1.0.1

---

## 12. Open questions / blockers as of 2026-05-02

- [ ] 🔴 **YOU:** Avid commercial AAX license — request not yet sent (§4.6 — do this today)
- [ ] 🔴 **YOU:** iLok License Manager installed and 2nd-gen iLok USB confirmed visible to OS
- [ ] 🔴 **YOU:** Apple Developer ID Application + Installer certs not yet generated (§3.1 / §3.2)
- [ ] 🔴 **YOU:** App-specific notarytool password not yet created (§3.3)
- [ ] 🔴 **YOU:** Windows VS2022 + JUCE clone not yet on the Windows laptop (§5.1)
- [ ] Decision: Path A (one-shot) vs Path B (staged) — defer until Avid response or 7 days, whichever first
