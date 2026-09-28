# Run interactively in PowerShell. Do not run through an agent or transcript.
$ErrorActionPreference = 'Stop'
$rclWrapTool = 'C:/Program Files/PACEAntiPiracy/Eden/Fusion/Versions/6/bin/wraptool.exe'
if (-not (Test-Path -LiteralPath $rclWrapTool)) { throw 'PACE signing tools were not found.' }
Write-Host 'Keep the developer iLok connected. This syncs PACE configuration; it does not sign a plugin.'
Write-Host 'PACE caches your credentials. The password briefly exists in the child process arguments.'
$rclAccount = Read-Host 'iLok User ID'
if ([string]::IsNullOrWhiteSpace($rclAccount)) { throw 'Cancelled: no account entered.' }
$rclSecurePassword = Read-Host 'iLok password' -AsSecureString
$rclPasswordPointer = [IntPtr]::Zero
try {
    $rclPasswordPointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($rclSecurePassword)
    $rclPassword = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($rclPasswordPointer)
    if ([string]::IsNullOrEmpty($rclPassword)) { throw 'No password entered.' }
    $rclOutput = & $rclWrapTool sync --account $rclAccount --password $rclPassword 2>&1
    $rclResult = $LASTEXITCODE
    foreach ($rclLine in $rclOutput) {
        Write-Host ($rclLine.ToString().Replace($rclPassword, '[REDACTED]'))
    }
    if ($rclResult -eq 0) { Write-Host 'PACE synchronization completed. Return to Codex.' }
    else { Write-Host "PACE synchronization failed (exit $rclResult). Share the error, never the password." }
} catch {
    # Exception text can contain process arguments; deliberately do not print it.
    Write-Host 'PACE setup failed. No exception details were printed to protect your password.'
} finally {
    $rclPassword = $null
    $rclOutput = $null
    if ($rclPasswordPointer -ne [IntPtr]::Zero) {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($rclPasswordPointer)
    }
    $rclSecurePassword.Dispose()
}
