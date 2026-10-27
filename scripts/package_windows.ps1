<#
.SYNOPSIS
    Packages IOneXa into a standalone, portable Windows x64 distribution.
.DESCRIPTION
    Deploys Qt 6 runtime dependencies via windeployqt, bundles Edge WebView2 loader,
    creates SHA256 checksums, and produces a portable zip archive.
#>

param (
    [string]$Version = "0.1.0",
    [string]$BuildDir = "build",
    [string]$OutputDir = "dist"
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "   IOneXa Windows x64 Packaging Engine (Usha Labs)" -ForegroundColor Cyan
Write-Host "   Version: v$Version" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$ExeSource = Join-Path $ProjectRoot "$BuildDir\ionexa.exe"
if (-not (Test-Path $ExeSource)) {
    Write-Error "Executable not found at '$ExeSource'. Build the project first."
}

$DistFolderName = "IOneXa-v$Version-windows-x64"
$StagingDir = Join-Path $ProjectRoot "$OutputDir\$DistFolderName"
$ZipTarget = Join-Path $ProjectRoot "$OutputDir\$DistFolderName.zip"

Write-Host "[1/5] Preparing staging directory at: $StagingDir" -ForegroundColor Yellow
if (Test-Path $StagingDir) {
    Remove-Item -Path $StagingDir -Recurse -Force
}
if (Test-Path $ZipTarget) {
    Remove-Item -Path $ZipTarget -Force
}
New-Item -Path $StagingDir -ItemType Directory -Force | Out-Null

Write-Host "[2/5] Staging application binary and WebView2 loader..." -ForegroundColor Yellow
Copy-Item -Path $ExeSource -Destination (Join-Path $StagingDir "ionexa.exe")
$WebView2Dll = Join-Path $ProjectRoot "vendor\webview2\bin\x64\WebView2Loader.dll"
if (Test-Path $WebView2Dll) {
    Copy-Item -Path $WebView2Dll -Destination (Join-Path $StagingDir "WebView2Loader.dll")
} else {
    Write-Warning "WebView2Loader.dll not found in vendor directory."
}

Write-Host "[3/5] Deploying Qt 6 runtime libraries with windeployqt..." -ForegroundColor Yellow
$WinDeployQt = "C:\Qt\6.8.2\msvc2022_64\bin\windeployqt.exe"
if (Test-Path $WinDeployQt) {
    & $WinDeployQt --no-translations --no-opengl-sw --release (Join-Path $StagingDir "ionexa.exe")
} else {
    Write-Warning "windeployqt.exe not found at standard path '$WinDeployQt'."
}

Write-Host "[4/5] Generating release metadata and documentation..." -ForegroundColor Yellow
$ReadmeContent = @"
================================================================================
IOneXa — Windows-Native Expo / React Native Developer Workbench
Version: $Version
Publisher: Usha Labs
Repository: https://github.com/ushalabs/IOneXa
================================================================================

HOW TO RUN:
1. Double-click 'ionexa.exe' to launch the workbench.
2. Click 'Open Project' in the Navigation Rail or File Menu.
3. Select any valid Expo application directory (e.g. containing package.json and app.json).
4. Click 'Start Preview' to launch Metro bundler and view the live interactive app.

SYSTEM REQUIREMENTS:
- Windows 11 x64 (or Windows 10 x64 version 1809+)
- Node.js v18.0.0 or newer with npm
- Microsoft Edge WebView2 Evergreen Runtime (preinstalled on modern Windows)

TRUTH IN ENGINEERING DISCLOSURE:
IOneXa executes Expo and React Native applications via Expo's supported Web
bundler target (Metro + react-native-web) embedded within Microsoft Edge
WebView2. It provides fast UI development, responsive viewports, and CDP
diagnostics. It is NOT an iOS emulator or macOS simulator. Final verification
of native iOS modules requires physical device or macOS Xcode execution.
"@

Set-Content -Path (Join-Path $StagingDir "README_PORTABLE.txt") -Value $ReadmeContent

# Generate SHA256 Checksums
Write-Host "[5/5] Generating checksums and creating ZIP archive..." -ForegroundColor Yellow
$Files = Get-ChildItem -Path $StagingDir -Recurse -File
$ChecksumLines = foreach ($f in $Files) {
    $hash = (Get-FileHash -Path $f.FullName -Algorithm SHA256).Hash.ToLower()
    $relPath = $f.FullName.Substring($StagingDir.Length + 1).Replace("\", "/")
    "$hash  $relPath"
}
Set-Content -Path (Join-Path $StagingDir "SHA256SUMS.txt") -Value $ChecksumLines

Compress-Archive -Path $StagingDir -DestinationPath $ZipTarget -CompressionLevel Optimal

$ZipSizeMb = [math]::Round((Get-Item $ZipTarget).Length / 1MB, 2)
Write-Host "==========================================================" -ForegroundColor Green
Write-Host "Packaging Complete!" -ForegroundColor Green
Write-Host "Target Archive: $ZipTarget ($ZipSizeMb MB)" -ForegroundColor Green
Write-Host "==========================================================" -ForegroundColor Green
