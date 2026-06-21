[Setup]
AppName=Vocal Lab
AppVersion=1.0.0
AppPublisher=CiptaSuara & Soba Studio
DefaultDirName={pf}\Vocal Lab
DefaultGroupName=Vocal Lab
OutputDir=.\Installer
OutputBaseFilename=VocalLab_Installer_v1.0.0
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=compiler:SetupClassicIcon.ico

[Types]
Name: "full"; Description: "Full installation (Recommended)"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 Plugin (64-bit)"; Types: full custom
Name: "standalone"; Description: "Standalone Application"; Types: full custom

[Files]
; Standalone Application
Source: "H:\DEV\Vst\VocalLab\build\VocalLab_artefacts\Release\Standalone\Vocal Lab.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion

; VST3 Plugin
Source: "H:\DEV\Vst\VocalLab\build\VocalLab_artefacts\Release\VST3\Vocal Lab.vst3\*"; DestDir: "{commoncf64}\VST3\Vocal Lab.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Vocal Lab"; Filename: "{app}\Vocal Lab.exe"; Components: standalone
Name: "{group}\Uninstall Vocal Lab"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Vocal Lab"; Filename: "{app}\Vocal Lab.exe"; Tasks: desktopicon; Components: standalone

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for Standalone"; GroupDescription: "Additional icons:"; Components: standalone
