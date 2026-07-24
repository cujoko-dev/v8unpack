[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string]$Archive,

    [Parameter(Mandatory)]
    [string]$ExpectedVersion
)

$ErrorActionPreference = "Stop"
$archivePath = (Resolve-Path -LiteralPath $Archive).Path
$workDirectory = Join-Path ([IO.Path]::GetTempPath()) ("v8unpack-portable-" + [guid]::NewGuid().ToString("N"))

try {
    Expand-Archive -LiteralPath $archivePath -DestinationPath $workDirectory
    $executable = Join-Path $workDirectory "v8unpack.exe"
    if (-not (Test-Path -LiteralPath $executable)) {
        throw "Archive does not contain v8unpack.exe"
    }

    $originalPath = $env:PATH
    try {
        # A portable release must start without DLLs from vcpkg or the build tree.
        $env:PATH = [Environment]::GetFolderPath([Environment+SpecialFolder]::System)
        $output = (& $executable --version 2>&1 | Out-String).Trim()
        $exitCode = $LASTEXITCODE
    }
    finally {
        $env:PATH = $originalPath
    }

    if ($exitCode -ne 0) {
        throw "Portable executable failed with exit code $exitCode"
    }
    if ($output -notmatch ('^' + [regex]::Escape($ExpectedVersion) + ' ')) {
        throw "Unexpected version output: $output"
    }

    Write-Output "Portable archive verified: $output"
}
finally {
    if (Test-Path -LiteralPath $workDirectory) {
        Remove-Item -LiteralPath $workDirectory -Recurse -Force
    }
}
