# Sign Lite AAX with PACE's local iLok and Azure Authenticode integration.
# Validate the unsigned bundle first. Credentials stay in the normal tool caches.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Bundle,
    [Parameter(Mandatory)][string]$OutputBundle,
    [Parameter(Mandatory)][string]$WrapTool,
    [Parameter(Mandatory)][string]$SignTool,
    [Parameter(Mandatory)][string]$Dlib,
    [Parameter(Mandatory)][string]$Metadata,
    [Parameter(Mandatory)][string]$AzureCliBin
)
$ErrorActionPreference = 'Stop'
$Bundle = (Resolve-Path -LiteralPath $Bundle).Path
$OutputBundle = [IO.Path]::GetFullPath($OutputBundle)
if (Test-Path -LiteralPath $OutputBundle) { throw "Refusing to overwrite $OutputBundle" }
foreach ($path in @($WrapTool, $SignTool, $Dlib, $Metadata)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing file: $path" }
}
$Dlib = (Resolve-Path -LiteralPath $Dlib).Path
$Metadata = (Resolve-Path -LiteralPath $Metadata).Path
foreach ($path in @($Dlib, $Metadata)) {
    if ($path -match '["\r\n]') { throw 'Unsupported character in signing option path.' }
}
if (-not (Test-Path -LiteralPath (Join-Path $AzureCliBin 'az.cmd'))) { throw 'Azure CLI not found.' }
$profile = Get-Content -LiteralPath (Join-Path $Bundle 'Contents/Resources/RCLBuildConfig.txt')
foreach ($line in @('version=2.0.1', 'moonbase=ON', 'final=ON', 'moonbase_sdk=4.4.0-rcl-hardening-1')) {
    if ($profile -cnotcontains $line) { throw "Final build marker missing: $line" }
}
$inputBinary = Join-Path $Bundle 'Contents/x64/Robin Control Lite.aaxplugin'
$version = (Get-Item -LiteralPath $inputBinary).VersionInfo
if ($version.ProductVersion -ne '2.0.1' -or $version.ProductName -ne 'Robin Control Lite') {
    throw 'This signing configuration is only for Robin Control Lite 2.0.1.'
}
New-Item -ItemType Directory -Path (Split-Path -Parent $OutputBundle) -Force | Out-Null
# Windows PACE requires identical input/output paths. Sign only a fresh copy.
Copy-Item -LiteralPath $Bundle -Destination $OutputBundle -Recurse -ErrorAction Stop
$env:PATH = "$AzureCliBin;$env:PATH"
$options = 'sign /v /fd SHA256 /tr http://timestamp.acs.microsoft.com /td SHA256 /dlib "' + $Dlib + '" /dmdf "' + $Metadata + '"'
& $WrapTool sign --wcguid 8F95C7F0-B538-11F1-8437-00505692AD3E --signtool $SignTool --explicitsigningoptions $options --in $OutputBundle --out $OutputBundle
if ($LASTEXITCODE -ne 0) { throw "PACE signing failed ($LASTEXITCODE). Preserve the output for diagnosis." }
$binary = Join-Path $OutputBundle 'Contents/x64/Robin Control Lite.aaxplugin'
& $WrapTool verify --in $binary
if ($LASTEXITCODE -ne 0) { throw 'PACE output verification failed.' }
& $SignTool verify /pa /all /v $binary
if ($LASTEXITCODE -ne 0) { throw 'AAX Authenticode verification failed.' }
$signature = Get-AuthenticodeSignature -LiteralPath $binary
if ($signature.Status -ne 'Valid' -or
    $signature.SignerCertificate.Subject -notmatch '(^|,\s*)CN=CONDUIT DSP LLC(,|$)' -or
    $null -eq $signature.TimeStamperCertificate) {
    throw 'Expected a valid, timestamped CONDUIT DSP LLC signature.'
}
foreach ($resource in @('RCLBuildConfig.txt', 'MoonbaseNotices.txt', 'RobinControlLitePages.xml')) {
    $relative = "Contents/Resources/$resource"
    if ((Get-FileHash -LiteralPath (Join-Path $Bundle $relative)).Hash -ne
        (Get-FileHash -LiteralPath (Join-Path $OutputBundle $relative)).Hash) {
        throw "Signing changed resource: $resource"
    }
}
Get-FileHash -LiteralPath $binary -Algorithm SHA256
