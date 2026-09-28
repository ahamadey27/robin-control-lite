; Private Windows x64 VST3 beta with Moonbase activation.
; Build: ISCC.exe installer\windows\RobinControlLiteBeta.iss
#define MyAppVersion "2.0.0"
#define MyBundle "..\..\NewProject\build-windows-beta-2.0.0\RobinControlLite_artefacts\Release\VST3\Robin Control Lite.vst3"

[Setup]
; Preserve the existing product identity for subsequent upgrades/uninstalls.
AppId={{8E1F12C7-3D4F-4E2A-A2D6-E5C92F4C7B3A}
AppName=Robin Control Lite
AppVersion={#MyAppVersion}
AppVerName=Robin Control Lite {#MyAppVersion} Moonbase Windows Beta 1
AppPublisher=Conduit DSP LLC
AppPublisherURL=https://conduitdsp.com
DefaultDirName={autopf}\Common Files\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\..\Releases\Installers\Beta
OutputBaseFilename=RobinControlLite-{#MyAppVersion}-Moonbase-Windows-Beta-1
PrivilegesRequired=admin
UninstallDisplayName=Robin Control Lite (Moonbase Windows Beta 1)
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
LicenseFile=..\..\EULA.md
InfoBeforeFile=beta-welcome.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#MyBundle}\*"; DestDir: "{commoncf}\VST3\Robin Control Lite.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\Robin Control Lite.vst3"
