#!/bin/bash
# Quick test script to run all examples with Xvfb
# This demonstrates that the build is working correctly

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

# Check if build directory exists
if [ ! -d "$BUILD_DIR/bin" ]; then
    echo "Error: Build directory not found. Please run CMake build first."
    echo "  mkdir build && cd build && cmake .. && cmake --build ."
    exit 1
fi

# Run each example for 2 seconds
EXAMPLES=(
    "HelloApp"
    "HelloIO"
    "HelloThreadLog"
    "HelloTriangle"
)

echo "=========================================="
echo "  Testing ToyFrameV Examples with Xvfb"
echo "=========================================="
echo ""

for example in "${EXAMPLES[@]}"; do
    echo "Running $example..."
    timeout 2 "$SCRIPT_DIR/run_with_xvfb.sh" "$BUILD_DIR/bin/$example" > /dev/null 2>&1
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 124 ] || [ $EXIT_CODE -eq 0 ]; then
        echo "  ✓ $example - OK"
    else
        echo "  ✗ $example - FAILED (exit code: $EXIT_CODE)"
    fi
done

echo ""
echo "=========================================="
echo "  All tests completed!"
echo "=========================================="
