<# 
.SYNOPSIS
    Build ToyFrameV for Windows using Visual Studio

.DESCRIPTION
    This script configures and builds ToyFrameV for Windows platform
    using CMake and Visual Studio.

.EXAMPLE
    .\build_windows.ps1
    .\build_windows.ps1 -Clean
    .\build_windows.ps1 -Config Debug
    .\build_windows.ps1 -Target HelloTriangle
#>

param(
    [switch]$Clean,                    # Clean build directory before building
    [string]$Config = "Release",       # Build configuration (Debug/Release)
    [string]$Target = "",              # Specific target to build (empty = all)
    [string]$Generator = "Visual Studio 17 2022"  # CMake generator
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectDir "build"

function Write-Header {
    param([string]$Text)
    Write-Host ""
    Write-Host "============================================" -ForegroundColor Cyan
    Write-Host "  $Text" -ForegroundColor Cyan
    Write-Host "============================================" -ForegroundColor Cyan
}

function Write-Step {
    param([string]$Text)
    Write-Host "[*] $Text" -ForegroundColor Green
}

function Write-Info {
    param([string]$Text)
    Write-Host "    $Text" -ForegroundColor Gray
}

# =============================================================================
# Main Build Process
# =============================================================================

Write-Header "Building ToyFrameV for Windows"
Write-Info "Project: $ProjectDir"
Write-Info "Build:   $BuildDir"
Write-Info "Config:  $Config"
Write-Info "Generator: $Generator"

# Clean if requested
if ($Clean) {
    Write-Step "Cleaning build directory..."
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Step "Creating build directory..."
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# Configure with CMake
Write-Step "Configuring with CMake..."
$cmakeArgs = @(
    "..",
    "-G", $Generator,
    "-A", "x64"
)

cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed"
}

# Build
Write-Step "Building..."
$buildArgs = @(
    "--build", ".",
    "--config", $Config
)

if ($Target -ne "") {
    $buildArgs += "--target"
    $buildArgs += $Target
}

cmake @buildArgs
if ($LASTEXITCODE -ne 0) {
    throw "Build failed"
}

Set-Location $ProjectDir

# Output locations
Write-Header "Build Complete!"
$binDir = Join-Path $BuildDir "bin\$Config"
if (Test-Path $binDir) {
    Write-Step "Output files:"
    Get-ChildItem $binDir -Filter "*.exe" | ForEach-Object {
        Write-Info $_.FullName
    }
}

Write-Host ""
Write-Host "To run a sample:" -ForegroundColor Yellow
Write-Host "  .\build\bin\$Config\HelloTriangle.exe" -ForegroundColor Gray
Write-Host ""
