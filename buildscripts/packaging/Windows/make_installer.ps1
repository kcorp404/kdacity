<#
.SYNOPSIS
    Stages the Kdacity dist tree and compiles the Windows installer.

.DESCRIPTION
    Runs `cmake --install` into the dist prefix (which is also where MSVC drops
    the built exe, see RUNTIME_OUTPUT_DIRECTORY in src/app/CMakeLists.txt) and
    then compiles buildscripts/packaging/Windows/Installer/Kdacity.iss with the
    Inno Setup command-line compiler.

    The upstream CPack/WiX path is used by CI and needs WiX Toolset v3.11;
    this script is the local equivalent and only needs Inno Setup 6.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File buildscripts\packaging\Windows\make_installer.ps1
#>
[CmdletBinding()]
param(
    [string]$BuildDir  = "build",
    [string]$DistDir   = "dist",
    [string]$Config    = "Release",
    [string]$OutputDir = "build.artifacts",
    # Skip the cmake install step and package whatever is already in DistDir.
    [switch]$SkipInstall
)

$ErrorActionPreference = "Stop"

$root = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
$buildPath  = if ([System.IO.Path]::IsPathRooted($BuildDir))  { $BuildDir }  else { Join-Path $root $BuildDir }
$distPath   = if ([System.IO.Path]::IsPathRooted($DistDir))   { $DistDir }   else { Join-Path $root $DistDir }
$outputPath = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $root $OutputDir }

# Version comes from version.cmake so the installer never drifts from the app.
$versionFile = Join-Path $root "version.cmake"
$vc = Get-Content $versionFile -Raw
function Get-VersionPart([string]$name) {
    $m = [regex]::Match($vc, "set\($name\s+`"([^`"]*)`"\)")
    if (-not $m.Success) { throw "Could not read $name from $versionFile" }
    return $m.Groups[1].Value
}
$appVersion = "{0}.{1}.{2}" -f (Get-VersionPart "MUSE_APP_VERSION_MAJOR"),
                               (Get-VersionPart "MUSE_APP_VERSION_MINOR"),
                               (Get-VersionPart "MUSE_APP_VERSION_PATCH")
Write-Host "[make_installer] version   : $appVersion"
Write-Host "[make_installer] dist      : $distPath"
Write-Host "[make_installer] output    : $outputPath"

if (-not $SkipInstall) {
    Write-Host "[make_installer] running cmake --install ..."
    & cmake --install $buildPath --config $Config --prefix $distPath
    if ($LASTEXITCODE -ne 0) { throw "cmake --install failed with exit code $LASTEXITCODE" }
}

# Sanity-check the tree before packaging. A dist without qml/ produces an
# installer whose app hangs on the splash screen, which is exactly the failure
# this check exists to catch.
$mustExist = @(
    "bin\Kdacity4.exe",
    "bin\qt.conf",
    "bin\platforms\qwindows.dll",
    "qml\QtQuick\qmldir",
    "qml\QtQuick\Window\qmldir",
    "qml\QtQml\qmldir"
)
$missing = @()
foreach ($rel in $mustExist) {
    if (-not (Test-Path (Join-Path $distPath $rel))) { $missing += $rel }
}
if ($missing.Count -gt 0) {
    throw "dist tree is incomplete, refusing to package. Missing:`n  " + ($missing -join "`n  ")
}
Write-Host "[make_installer] dist tree looks complete."

$iscc = $null
foreach ($candidate in @(
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "$env:ProgramFiles\Inno Setup 6\ISCC.exe"
)) {
    if (Test-Path $candidate) { $iscc = $candidate; break }
}
if (-not $iscc) {
    $cmd = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($cmd) { $iscc = $cmd.Source }
}
if (-not $iscc) { throw "ISCC.exe (Inno Setup 6) not found. Install it from https://jrsoftware.org/isdl.php" }
Write-Host "[make_installer] ISCC      : $iscc"

New-Item -ItemType Directory -Force -Path $outputPath | Out-Null

$iss = Join-Path $root "buildscripts\packaging\Windows\Installer\Kdacity.iss"
& $iscc "/DSourceRoot=$root" "/DDistDir=$distPath" "/DAppVersion=$appVersion" "/DOutputDir=$outputPath" $iss
if ($LASTEXITCODE -ne 0) { throw "ISCC failed with exit code $LASTEXITCODE" }

$setup = Join-Path $outputPath ("Kdacity-{0}-x86_64-setup.exe" -f $appVersion)
Write-Host ""
Write-Host "[make_installer] Installer: $setup"
Write-Host ("[make_installer] Size     : {0:N1} MB" -f ((Get-Item $setup).Length / 1MB))
