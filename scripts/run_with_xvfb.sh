#!/bin/bash
# Script to run ToyFrameV applications with Xvfb (virtual framebuffer)
# Usage: ./run_with_xvfb.sh <program> [args...]

# Default display number
DISPLAY_NUM=99

# Check if Xvfb is installed
if ! command -v Xvfb &> /dev/null; then
    echo "Error: Xvfb is not installed"
    echo "Install with: sudo apt-get install -y xvfb"
    exit 1
fi

# Check if a display is already running
if pgrep -f "Xvfb :${DISPLAY_NUM}" > /dev/null; then
    echo "Xvfb already running on display :${DISPLAY_NUM}"
else
    echo "Starting Xvfb on display :${DISPLAY_NUM}..."
    Xvfb :${DISPLAY_NUM} -screen 0 1280x720x24 > /dev/null 2>&1 &
    XVFB_PID=$!
    sleep 2
    
    # Check if Xvfb started successfully
    if ! pgrep -f "Xvfb :${DISPLAY_NUM}" > /dev/null; then
        echo "Error: Failed to start Xvfb"
        exit 1
    fi
    echo "Xvfb started successfully (PID: $XVFB_PID)"
fi

# Run the command with DISPLAY set
echo "Running: $@"
echo "=========================================="
DISPLAY=:${DISPLAY_NUM} "$@"
EXIT_CODE=$?

echo "=========================================="
echo "Program exited with code: $EXIT_CODE"

# Note: We don't kill Xvfb here so it can be reused
# To stop Xvfb manually: pkill Xvfb

exit $EXIT_CODE
