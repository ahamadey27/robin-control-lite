# Uses the caller's Azure CLI session, never a key file.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$File,
    [Parameter(Mandatory)][string]$SignTool,
    [Parameter(Mandatory)][string]$Dlib,
    [Parameter(Mandatory)][string]$Metadata,
    [Parameter(Mandatory)][string]$AzureCliBin
)
$ErrorActionPreference = 'Stop'
try {
    foreach ($path in @($File, $SignTool, $Dlib, $Metadata)) {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing file: $path" }
    }
    if (-not (Test-Path -LiteralPath (Join-Path $AzureCliBin 'az.cmd'))) { throw 'Azure CLI not found.' }
    $env:PATH = "$AzureCliBin;$env:PATH"
    & $SignTool sign /v /fd SHA256 /tr http://timestamp.acs.microsoft.com /td SHA256 /dlib $Dlib /dmdf $Metadata $File
    if ($LASTEXITCODE -ne 0) { throw "Signing failed ($LASTEXITCODE)." }
    & $SignTool verify /pa /all /v $File
    if ($LASTEXITCODE -ne 0) { throw 'SignTool verification failed.' }
    $signature = Get-AuthenticodeSignature -LiteralPath $File
    if ($signature.Status -ne 'Valid' -or
        $signature.SignerCertificate.Subject -notmatch '(^|,\s*)CN=CONDUIT DSP LLC(,|$)' -or
        $null -eq $signature.TimeStamperCertificate) {
        throw 'Expected a valid, timestamped CONDUIT DSP LLC signature.'
    }
} catch {
    Write-Error $_
    exit 1
}
