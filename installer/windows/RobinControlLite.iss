; Final Windows x64 VST3 and optional AAX installer. Invoke through build-final.ps1, which checks
; the DRM/final markers, notices, publisher signature and overwrite protection.
#define MyAppVersion "2.0.1"
#ifndef MyBundle
  #error Use build-final.ps1 with a validated signed final VST3 bundle.
#endif

[Setup]
; Preserve the product AppId for upgrades from existing releases/betas.
AppId={{8E1F12C7-3D4F-4E2A-A2D6-E5C92F4C7B3A}
AppName=Robin Control Lite
AppVersion={#MyAppVersion}
AppVerName=Robin Control Lite {#MyAppVersion}
AppPublisher=Conduit DSP LLC
AppPublisherURL=https://conduitdsp.com
AppSupportURL=https://conduitdsp.com
AppUpdatesURL=https://conduitdsp.com
DefaultDirName={autopf}\Common Files\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\..\Releases\Installers
OutputBaseFilename=RobinControlLite-{#MyAppVersion}-Windows
PrivilegesRequired=admin
UninstallDisplayName=Robin Control Lite
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
LicenseFile=..\..\EULA.md
SignTool=azure
SignedUninstaller=yes
SignToolRunMinimized=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#MyBundle}\*"; DestDir: "{commoncf}\VST3\Robin Control Lite.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
#ifdef MyAaxBundle
Source: "{#MyAaxBundle}\*"; DestDir: "{commoncf}\Avid\Audio\Plug-Ins\Robin Control Lite.aaxplugin"; Flags: recursesubdirs createallsubdirs ignoreversion
#endif

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\Robin Control Lite.vst3"
#ifdef MyAaxBundle
Type: filesandordirs; Name: "{commoncf}\Avid\Audio\Plug-Ins\Robin Control Lite.aaxplugin"
#endif
