#!/bin/bash
#
# Build ToyFrameV for WebGL using Emscripten
#
# This script automatically downloads and sets up Emscripten SDK locally,
# then builds ToyFrameV for WebGL. The emsdk is kept in a local directory
# and is excluded from git.
#
# Usage:
#   ./build_web.sh          # Build
#   ./build_web.sh --clean  # Clean and build
#   ./build_web.sh --serve  # Build and start server
#   ./build_web.sh --setup  # Only setup emsdk

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EMSDK_DIR="$SCRIPT_DIR/emsdk"
BUILD_DIR="$SCRIPT_DIR/build-web"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

# Parse arguments
CLEAN=false
SERVE=false
SETUP_ONLY=false

for arg in "$@"; do
    case $arg in
        --clean) CLEAN=true ;;
        --serve) SERVE=true ;;
        --setup) SETUP_ONLY=true ;;
    esac
done

print_header() {
    echo ""
    echo -e "${CYAN}============================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}============================================${NC}"
}

print_step() {
    echo -e "${GREEN}[*] $1${NC}"
}

print_info() {
    echo -e "${GRAY}    $1${NC}"
}

print_error() {
    echo -e "${RED}ERROR: $1${NC}"
}

# =============================================================================
# Setup Emscripten SDK
# =============================================================================

setup_emsdk() {
    print_header "Setting up Emscripten SDK"
    
    if [ -f "$EMSDK_DIR/emsdk" ]; then
        print_step "Emsdk already installed at: $EMSDK_DIR"
    else
        print_step "Downloading Emscripten SDK..."
        git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
    fi
    
    cd "$EMSDK_DIR"
    
    print_step "Installing latest Emscripten..."
    ./emsdk install latest
    
    print_step "Activating Emscripten..."
    ./emsdk activate latest
    
    cd "$SCRIPT_DIR"
    echo -e "${GREEN}Emsdk setup complete!${NC}"
}

# =============================================================================
# Activate Emscripten environment
# =============================================================================

activate_emsdk() {
    print_step "Activating Emscripten environment..."
    
    if [ ! -f "$EMSDK_DIR/emsdk_env.sh" ]; then
        print_error "emsdk_env.sh not found. Run with --setup first."
        exit 1
    fi
    
    source "$EMSDK_DIR/emsdk_env.sh"
    
    if ! command -v emcmake &> /dev/null; then
        print_error "emcmake not found after activation."
        exit 1
    fi
    
    print_info "Emscripten activated: $EMSDK"
}

# =============================================================================
# Build
# =============================================================================

build_web() {
    print_header "Building ToyFrameV for WebGL"
    
    # Clean if requested
    if [ "$CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
        print_step "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_step "Configuring CMake..."
    emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
    
    print_step "Building..."
    cmake --build . --config Release -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    cd "$SCRIPT_DIR"
    
    print_header "Build Complete!"
    echo ""
    echo "  Output files:"
    
    if [ -d "$BUILD_DIR/bin" ]; then
        for f in "$BUILD_DIR/bin"/*.html; do
            [ -f "$f" ] && echo -e "    - ${YELLOW}$(basename $f)${NC}"
        done
    fi
    
    echo ""
    echo "  To test locally:"
    echo -e "    ${YELLOW}./build_web.sh --serve${NC}"
    echo ""
}

# =============================================================================
# Serve
# =============================================================================

start_server() {
    local BIN_DIR="$BUILD_DIR/bin"
    
    if [ ! -d "$BIN_DIR" ]; then
        print_error "Build output not found. Run build first."
        exit 1
    fi
    
    print_header "Starting Local Server"
    echo ""
    echo -e "  Open in browser: ${YELLOW}http://localhost:8080/HelloTriangle.html${NC}"
    echo -e "  ${GRAY}Press Ctrl+C to stop${NC}"
    echo ""
    
    cd "$BIN_DIR"
    python3 -m http.server 8080 || python -m http.server 8080
}

# =============================================================================
# Main
# =============================================================================

print_header "ToyFrameV WebGL Builder"

# Always ensure emsdk is set up
setup_emsdk

if [ "$SETUP_ONLY" = true ]; then
    echo -e "${GREEN}Setup complete. Use ./build_web.sh to build.${NC}"
    exit 0
fi

# Activate environment
activate_emsdk

if [ "$SERVE" = true ]; then
    build_web
    start_server
else
    build_web
fi
