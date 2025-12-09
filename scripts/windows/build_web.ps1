<# 
.SYNOPSIS
    Build ToyFrameV for WebGL using Emscripten

.DESCRIPTION
    This script automatically downloads and sets up Emscripten SDK locally,
    then builds ToyFrameV for WebGL. The emsdk is kept in a local directory
    and is excluded from git.

.EXAMPLE
    .\scripts\build_web.ps1
    .\scripts\build_web.ps1 -Clean
    .\scripts\build_web.ps1 -Serve
    .\scripts\build_web.ps1 -Target HelloTriangle
#>

param(
    [switch]$Clean,       # Clean build directory before building
    [switch]$Serve,       # Build and start a local server
    [switch]$Setup,       # Only setup emsdk, don't build
    [string]$Target = "" # Specific target to build (empty = all)
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$EmsdkDir = Join-Path $ProjectDir "emsdk"
$BuildDir = Join-Path $ProjectDir "build-web"

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
# Setup Emscripten SDK
# =============================================================================

function Setup-Emsdk {
    Write-Header "Setting up Emscripten SDK"
    
    $emsdkExe = Join-Path $EmsdkDir "emsdk.bat"
    $emccPath = Join-Path $EmsdkDir "upstream\emscripten\emcc.bat"
    
    # Check if emsdk already exists
    if (Test-Path $emsdkExe) {
        Write-Step "Emsdk already installed at: $EmsdkDir"
    } else {
        Write-Step "Downloading Emscripten SDK..."
        
        # Clone emsdk
        git clone https://github.com/emscripten-core/emsdk.git $EmsdkDir
        if ($LASTEXITCODE -ne 0) {
            throw "Failed to clone emsdk"
        }
    }
    
    # Check if emcc exists (meaning latest is already installed)
    if (Test-Path $emccPath) {
        Write-Step "Emscripten already installed, skipping install/activate"
        return
    }
    
    Set-Location $EmsdkDir
    
    Write-Step "Installing latest Emscripten..."
    & $emsdkExe install latest
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to install emsdk"
    }
    
    Write-Step "Activating Emscripten..."
    & $emsdkExe activate latest
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to activate emsdk"
    }
    
    Set-Location $ProjectDir
    Write-Host "Emsdk setup complete!" -ForegroundColor Green
}

# =============================================================================
# Activate Emscripten environment (temporary, for this session only)
# =============================================================================

function Activate-Emsdk {
    Write-Step "Activating Emscripten environment..."
    
    $envScript = Join-Path $EmsdkDir "emsdk_env.bat"
    if (-not (Test-Path $envScript)) {
        throw "emsdk_env.bat not found. Run with -Setup first."
    }
    
    # Parse the emsdk_env.bat output to get environment variables
    $envOutput = cmd /c "`"$envScript`" && set"
    
    foreach ($line in $envOutput) {
        if ($line -match "^([^=]+)=(.*)$") {
            $varName = $matches[1]
            $varValue = $matches[2]
            
            # Only set EMSDK related variables and PATH
            if ($varName -like "EMSDK*" -or $varName -eq "PATH" -or $varName -eq "EM_CONFIG") {
                [Environment]::SetEnvironmentVariable($varName, $varValue, "Process")
            }
        }
    }
    
    # Verify emcmake is available
    $emcmake = Get-Command emcmake -ErrorAction SilentlyContinue
    if (-not $emcmake) {
        throw "emcmake not found after activation. Something went wrong."
    }
    
    Write-Info "Emscripten activated: $($env:EMSDK)"
}

# =============================================================================
# Build
# =============================================================================

function Build-Web {
    Write-Header "Building ToyFrameV for WebGL"
    
    # Clean if requested
    if ($Clean -and (Test-Path $BuildDir)) {
        Write-Step "Cleaning build directory..."
        Remove-Item -Recurse -Force $BuildDir
    }
    
    # Create build directory
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }
    
    Set-Location $BuildDir
    
    # Configure
    Write-Step "Configuring CMake..."
    emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    # Build
    Write-Step "Building..."
    $numCores = [Environment]::ProcessorCount
    $buildArgs = @("--build", ".", "--config", "Release", "-j", $numCores)
    if ($Target -ne "") {
        $buildArgs += "--target"
        $buildArgs += $Target
    }
    cmake @buildArgs
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    Set-Location $ProjectDir
    
    Write-Header "Build Complete!"
    Write-Host ""
    Write-Host "  Output files:" -ForegroundColor White
    
    $binDir = Join-Path $BuildDir "bin"
    if (Test-Path $binDir) {
        Get-ChildItem $binDir -Filter "*.html" | ForEach-Object {
            Write-Host "    - $($_.Name)" -ForegroundColor Yellow
        }
    }
    
    Write-Host ""
    Write-Host "  To test locally:" -ForegroundColor White
    Write-Host "    .\scripts\build_web.ps1 -Serve" -ForegroundColor Yellow
    Write-Host ""
}

# =============================================================================
# Serve
# =============================================================================

function Start-Server {
    $binDir = Join-Path $BuildDir "bin"
    
    if (-not (Test-Path $binDir)) {
        throw "Build output not found. Run build first."
    }
    
    Write-Header "Starting Local Server"
    Write-Host ""
    Write-Host "  Open in browser: " -NoNewline
    Write-Host "http://localhost:8080/HelloTriangle.html" -ForegroundColor Yellow
    Write-Host "  Press Ctrl+C to stop" -ForegroundColor Gray
    Write-Host ""
    
    Set-Location $binDir
    python -m http.server 8080
}

# =============================================================================
# Main
# =============================================================================

try {
    Write-Header "ToyFrameV WebGL Builder"
    
    # Always ensure emsdk is set up
    Setup-Emsdk
    
    if ($Setup) {
        Write-Host "Setup complete. Use .\build_web.ps1 to build." -ForegroundColor Green
        exit 0
    }
    
    # Activate environment
    Activate-Emsdk
    
    if ($Serve) {
        Build-Web
        Start-Server
    } else {
        Build-Web
    }
    
} catch {
    Write-Host ""
    Write-Host "ERROR: $_" -ForegroundColor Red
    exit 1
} finally {
    Set-Location $ProjectDir
}
