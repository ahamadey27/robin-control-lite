; Inno Setup script for Robin Control Lite v1.0 (Windows VST3-only, Path B).
; Build the installer with:
;   iscc installer\windows\RobinControlLite.iss
; Output: Releases\Installers\RobinControlLite-1.0.0-Windows.exe
;
; AAX is deferred to v1.0.1 per release-spec.md §1 (Path B). When AAX returns,
; uncomment the AAX [Files] entry below.

#define MyAppName       "Robin Control Lite"
#define MyAppVersion    "1.0.0"
#define MyAppPublisher  "Conduit DSP LLC"
#define MyAppURL        "https://conduitdsp.com"
; Build dir under NewProject/. release-spec §8.1 uses "build-release" for the
; official release pipeline; "build" is the dev configure dir from §5.3. Switch
; here to point the installer at whichever tree you want to package.
#define MyBuildDir      "build-release"

[Setup]
; Stable AppId — never change this between versions, or upgrades won't replace
; the prior install.
AppId={{8E1F12C7-3D4F-4E2A-A2D6-E5C92F4C7B3A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\Common Files\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\..\Releases\Installers
OutputBaseFilename=RobinControlLite-{#MyAppVersion}-Windows
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName}
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
LicenseFile=..\..\EULA.md

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; VST3 bundle → C:\Program Files\Common Files\VST3\Robin Control Lite.vst3\
Source: "..\..\NewProject\{#MyBuildDir}\RobinControlLite_artefacts\Release\VST3\Robin Control Lite.vst3\*"; \
  DestDir: "{commoncf}\VST3\Robin Control Lite.vst3"; \
  Flags: recursesubdirs createallsubdirs ignoreversion

; AAX (commented for v1.0; un-comment for v1.0.1 once PACE-signed):
; Source: "..\..\NewProject\{#MyBuildDir}\RobinControlLite_artefacts\Release\AAX\Robin Control Lite.aaxplugin\*"; \
;   DestDir: "{commoncf}\Avid\Audio\Plug-Ins\Robin Control Lite.aaxplugin"; \
;   Flags: recursesubdirs createallsubdirs ignoreversion

[UninstallDelete]
; Defensive cleanup — removes the bundle dir even if something inside it was
; created post-install (e.g. a regenerated moduleinfo.json) and isn't tracked
; by Inno's per-file uninstall log.
Type: filesandordirs; Name: "{commoncf}\VST3\Robin Control Lite.vst3"
