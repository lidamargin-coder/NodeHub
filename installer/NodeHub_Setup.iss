; Script generated for NodeHub - BeamNG.drive Mod Launcher
; Inno Setup 6.x

#define MyAppName "NodeHub"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "NodeHub"
#define MyAppExeName "NodeHub.exe"
#define MyDistDir "..\dist\NodeHub"

[Setup]
AppId={{5E2A8C19-4871-4A56-A833-289FA71C8F44}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppComments=BeamNG.drive Mod Launcher & Manager
AppCopyright=Copyright (C) 2026 NodeHub

; Default install to 64-bit Program Files: C:\Program Files\NodeHub
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes

; Administrator privileges required to install to Program Files
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

; Output file settings
OutputDir=..\dist
OutputBaseFilename=NodeHub_Setup_v1.1.0
SetupIconFile=..\logos\app.ico
UninstallDisplayIcon={app}\{#MyAppExeName}

; Ultra high compression
Compression=lzma2/ultra64
SolidCompression=yes

; Modern visual style and custom branding
WizardStyle=modern
WizardImageFile=installer_banner.bmp
WizardSmallImageFile=installer_small.bmp

; Privacy policy & STMods disclaimer agreement
LicenseFile=LICENSE_AND_DISCLAIMER.txt

; Clean uninstall registration
UninstallDisplayName={#MyAppName} (BeamNG.drive Mod Launcher)
CloseApplications=force

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#MyDistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall runasoriginaluser

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
