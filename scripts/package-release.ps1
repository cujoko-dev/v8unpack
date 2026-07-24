[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string]$Executable,

    [Parameter(Mandatory)]
    [string]$Version,

    [string]$OutputDirectory = "dist"
)

$ErrorActionPreference = "Stop"
$executablePath = (Resolve-Path -LiteralPath $Executable).Path
$repositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$outputPath = Join-Path $repositoryRoot $OutputDirectory
$stagingPath = Join-Path $outputPath "v8unpack-$Version-win-x64-built-by-cujoko"
$archivePath = "$stagingPath.zip"

New-Item -ItemType Directory -Path $outputPath -Force | Out-Null
if (Test-Path -LiteralPath $stagingPath) {
    Remove-Item -LiteralPath $stagingPath -Recurse -Force
}
if (Test-Path -LiteralPath $archivePath) {
    Remove-Item -LiteralPath $archivePath -Force
}
New-Item -ItemType Directory -Path $stagingPath | Out-Null

Copy-Item -LiteralPath $executablePath -Destination (Join-Path $stagingPath "v8unpack.exe")
Copy-Item -LiteralPath (Join-Path $repositoryRoot "README.md") -Destination $stagingPath
Copy-Item -LiteralPath (Join-Path $repositoryRoot "LICENSE") -Destination $stagingPath

$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $stagingPath "v8unpack.exe")).Hash.ToLowerInvariant()
Set-Content -LiteralPath (Join-Path $stagingPath "SHA256SUMS.txt") -Encoding ascii -Value "$hash  v8unpack.exe"

Compress-Archive -Path (Join-Path $stagingPath "*") -DestinationPath $archivePath -CompressionLevel Optimal
Remove-Item -LiteralPath $stagingPath -Recurse -Force

Write-Output $archivePath
