; Inno Setup script for Kdacity (Audacity 4 fork)
;
; Build with:
;   ISCC.exe /DSourceRoot=<repo> /DDistDir=<dist> /DAppVersion=4.0.0 Kdacity.iss
; All defines below have defaults so it can also be compiled directly from the IDE.

#ifndef SourceRoot
  #define SourceRoot "..\..\..\.."
#endif
#ifndef DistDir
  #define DistDir SourceRoot + "\dist"
#endif
#ifndef AppVersion
  #define AppVersion "4.0.0"
#endif
#ifndef OutputDir
  #define OutputDir SourceRoot + "\build.artifacts"
#endif

#define AppName        "Kdacity"
#define AppVerName     "Kdacity 4"
#define AppPublisher   "K-CORP"
#define AppExeName     "Kdacity4.exe"
#define AppExePath     "bin\" + AppExeName
#define ProgId         "Kdacity.aup4"

[Setup]
; Stable per-product GUID. Changing it makes Windows treat a new build as a
; separate product instead of an upgrade, so keep it fixed across releases.
AppId={{7B3F2C14-9A6D-4E58-B0C7-1D5E8F42A931}
AppName={#AppVerName}
AppVersion={#AppVersion}
AppVerName={#AppVerName} {#AppVersion}
VersionInfoVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\{#AppVerName}
DefaultGroupName={#AppVerName}
UninstallDisplayName={#AppVerName} {#AppVersion}
UninstallDisplayIcon={app}\{#AppExePath}
LicenseFile={#SourceRoot}\buildscripts\packaging\Windows\Installer\LICENSE.rtf
SetupIconFile={#SourceRoot}\share\icons\AppIcon\AU4_AppIcon.ico
OutputDir={#OutputDir}
OutputBaseFilename={#AppName}-{#AppVersion}-x86_64-setup
; The payload is ~200 MB of mostly-compressible DLLs and QML; lzma2/max keeps
; the installer roughly the size of the equivalent 7z.
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Default to a machine-wide install, but let the user drop to a per-user one
; (installs into {localappdata}) when they have no admin rights.
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline dialog
WizardStyle=modern
DisableProgramGroupPage=yes
ShowLanguageDialog=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "associate";   Description: "Associate .aup4 project files with {#AppVerName}"; GroupDescription: "File associations:"; Flags: unchecked

[Files]
; The whole dist tree ships as-is: bin\qt.conf points Qt at the parent
; directory, so qml\ and the sibling folders must keep their relative layout.
; bin\Release is the MSVC build-output staging copy of the exe (CMake sets
; RUNTIME_OUTPUT_DIRECTORY to <prefix>/bin), not part of the runtime.
Source: "{#DistDir}\*"; DestDir: "{app}"; \
    Excludes: "bin\Release\*,bin\Release,*.pdb,*.ilk,*.exp,*.lib"; \
    Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\{#AppVerName}";              Filename: "{app}\{#AppExePath}"; WorkingDir: "{app}\bin"
Name: "{group}\{cm:UninstallProgram,{#AppVerName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppVerName}";        Filename: "{app}\{#AppExePath}"; WorkingDir: "{app}\bin"; Tasks: desktopicon

[Registry]
Root: HKA; Subkey: "Software\Classes\.aup4"; ValueType: string; ValueName: ""; ValueData: "{#ProgId}"; \
    Flags: uninsdeletevalue; Tasks: associate
Root: HKA; Subkey: "Software\Classes\{#ProgId}"; ValueType: string; ValueName: ""; ValueData: "Kdacity Project"; \
    Flags: uninsdeletekey; Tasks: associate
Root: HKA; Subkey: "Software\Classes\{#ProgId}\DefaultIcon"; ValueType: string; ValueName: ""; \
    ValueData: "{app}\{#AppExePath},0"; Tasks: associate
Root: HKA; Subkey: "Software\Classes\{#ProgId}\shell\open\command"; ValueType: string; ValueName: ""; \
    ValueData: """{app}\{#AppExePath}"" ""%1"""; Tasks: associate

[Run]
Filename: "{app}\{#AppExePath}"; WorkingDir: "{app}\bin"; \
    Description: "{cm:LaunchProgram,{#AppVerName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Qt's shader/pipeline cache and other files the app writes under its own
; install dir are not tracked by the installer; drop the folder if it is empty.
Type: dirifempty; Name: "{app}\bin"
Type: dirifempty; Name: "{app}"
