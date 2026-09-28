# Package separately staged and signed final plugins. Run after validation.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Bundle,
    [Parameter(Mandatory)][string]$OutputDirectory,
    [Parameter(Mandatory)][string]$SignTool,
    [Parameter(Mandatory)][string]$Dlib,
    [Parameter(Mandatory)][string]$Metadata,
    [Parameter(Mandatory)][string]$AzureCliBin,
    [string]$AaxBundle,
    [string]$WrapTool,
    [string]$Compiler = 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe'
)
$ErrorActionPreference = 'Stop'
$Bundle = (Resolve-Path -LiteralPath $Bundle).Path
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$output = Join-Path $OutputDirectory 'RobinControlLite-2.0.0-Windows.exe'
if (Test-Path -LiteralPath $output) { throw "Refusing to overwrite $output" }
$profile = Get-Content -LiteralPath (Join-Path $Bundle 'Contents/Resources/RCLBuildConfig.txt')
foreach ($line in @('version=2.0.0', 'moonbase=ON', 'final=ON', 'moonbase_sdk=4.4.0-rcl-hardening-1')) {
    if ($profile -cnotcontains $line) { throw "Final build marker missing: $line" }
}
$notices = Join-Path $Bundle 'Contents/Resources/MoonbaseNotices.txt'
if (-not (Test-Path -LiteralPath $notices) -or (Get-Item -LiteralPath $notices).Length -eq 0) {
    throw 'Moonbase notices missing or empty.'
}
$binary = Join-Path $Bundle 'Contents/x86_64-win/Robin Control Lite.vst3'
$version = (Get-Item -LiteralPath $binary).VersionInfo
if ($version.ProductVersion -ne '2.0.0' -or $version.ProductName -ne 'Robin Control Lite') {
    throw 'VST3 executable identity/version does not match the installer.'
}
$signature = Get-AuthenticodeSignature -LiteralPath $binary
if ($signature.Status -ne 'Valid' -or
    $signature.SignerCertificate.Subject -notmatch '(^|,\s*)CN=CONDUIT DSP LLC(,|$)' -or
    $null -eq $signature.TimeStamperCertificate) { throw 'VST3 must have a valid timestamped CONDUIT DSP LLC signature.' }
& $SignTool verify /pa /all /v $binary
if ($LASTEXITCODE -ne 0) { throw 'VST3 signature verification failed.' }
$signScript = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../scripts/sign-windows-artifact.ps1')).Path
$shell = (Get-Process -Id $PID).Path
$aaxDefines = @()
if ($AaxBundle) {
    $AaxBundle = (Resolve-Path -LiteralPath $AaxBundle).Path
    if (-not $WrapTool -or -not (Test-Path -LiteralPath $WrapTool -PathType Leaf)) {
        throw 'AAX packaging requires WrapTool for signature verification.'
    }
    $aaxProfile = Get-Content -LiteralPath (Join-Path $AaxBundle 'Contents/Resources/RCLBuildConfig.txt')
    foreach ($line in @('version=2.0.0', 'moonbase=ON', 'final=ON', 'moonbase_sdk=4.4.0-rcl-hardening-1')) {
        if ($aaxProfile -cnotcontains $line) { throw "AAX final build marker missing: $line" }
    }
    foreach ($resource in @('MoonbaseNotices.txt', 'RobinControlLitePages.xml')) {
        $resourcePath = Join-Path $AaxBundle "Contents/Resources/$resource"
        if (-not (Test-Path -LiteralPath $resourcePath) -or (Get-Item -LiteralPath $resourcePath).Length -eq 0) {
            throw "AAX resource missing or empty: $resource"
        }
    }
    $aaxBinary = Join-Path $AaxBundle 'Contents/x64/Robin Control Lite.aaxplugin'
    $aaxVersion = (Get-Item -LiteralPath $aaxBinary).VersionInfo
    if ($aaxVersion.ProductVersion -ne '2.0.0' -or $aaxVersion.ProductName -ne 'Robin Control Lite') {
        throw 'AAX executable identity/version does not match the installer.'
    }
    & $WrapTool verify --in $aaxBinary
    if ($LASTEXITCODE -ne 0) { throw 'AAX PACE signature verification failed.' }
    & $SignTool verify /pa /all /v $aaxBinary
    if ($LASTEXITCODE -ne 0) { throw 'AAX Authenticode verification failed.' }
    $aaxSignature = Get-AuthenticodeSignature -LiteralPath $aaxBinary
    if ($aaxSignature.Status -ne 'Valid' -or
        $aaxSignature.SignerCertificate.Subject -notmatch '(^|,\s*)CN=CONDUIT DSP LLC(,|$)' -or
        $null -eq $aaxSignature.TimeStamperCertificate) {
        throw 'AAX must have a valid timestamped CONDUIT DSP LLC signature.'
    }
    $aaxDefines = @("/DMyAaxBundle=$AaxBundle")
}
foreach ($path in @($SignTool, $Dlib, $Metadata, $Compiler)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing file: $path" }
}
# Inno's $q/$f syntax handles spaces; reject its metacharacters in inputs.
foreach ($path in @($shell, $signScript, $SignTool, $Dlib, $Metadata, $AzureCliBin)) {
    if ($path -match '["$\r\n]') { throw 'Signing tool paths contain unsupported characters.' }
}
$command = '$q' + $shell + '$q -NoProfile -NonInteractive -ExecutionPolicy Bypass -File $q' + $signScript + '$q'
foreach ($pair in @(@('SignTool', $SignTool), @('Dlib', $Dlib), @('Metadata', $Metadata), @('AzureCliBin', $AzureCliBin))) {
    $command += ' -' + $pair[0] + ' $q' + $pair[1] + '$q'
}
$command += ' -File $f'
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
& $Compiler "/DMyBundle=$Bundle" @aaxDefines "/O$OutputDirectory" "/Sazure=$command" (Join-Path $PSScriptRoot 'RobinControlLite.iss')
if ($LASTEXITCODE -ne 0) { throw "Installer compilation/signing failed ($LASTEXITCODE)." }
& $SignTool verify /pa /all /v $output
if ($LASTEXITCODE -ne 0) { throw 'Installer verification failed.' }
$signature = Get-AuthenticodeSignature -LiteralPath $output
if ($signature.Status -ne 'Valid' -or
    $signature.SignerCertificate.Subject -notmatch '(^|,\s*)CN=CONDUIT DSP LLC(,|$)' -or
    $null -eq $signature.TimeStamperCertificate) { throw 'Installer signature identity/timestamp check failed.' }
Get-FileHash -LiteralPath $output -Algorithm SHA256
