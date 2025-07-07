#!/bin/bash

# Build script for compiling Iforets_on_OPS-SAT.cpp for gem5 v21.2.1.0
# This script handles cross-compilation and system call compatibility issues

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build_gem5_v21"

echo "=== Building Isolation Forest for gem5 v21.2.1.0 ==="
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"

# Create build directory
mkdir -p "$BUILD_DIR"

# Check if we have the required cross-compiler
if ! command -v arm-linux-gnueabi-g++ &> /dev/null; then
    echo "Error: arm-linux-gnueabi-g++ not found. Installing..."
    sudo apt-get update
    sudo apt-get install -y gcc-arm-linux-gnueabi g++-arm-linux-gnueabi
fi

# Set cross-compilation environment variables
export CC=arm-linux-gnueabi-gcc
export CXX=arm-linux-gnueabi-g++
export AR=arm-linux-gnueabi-ar
export STRIP=arm-linux-gnueabi-strip

# Compiler flags for gem5 v21.2.1.0 compatibility
CXXFLAGS="-std=c++17 -Wall -Wextra -O2 -static"
CXXFLAGS="$CXXFLAGS -DMLPACK_DISABLE_BFD_DL"  # Disable backtrace features that may cause issues
CXXFLAGS="$CXXFLAGS -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard"  # ARM7 compatibility
CXXFLAGS="$CXXFLAGS -fno-stack-protector"  # Disable stack protection for gem5 compatibility
CXXFLAGS="$CXXFLAGS -D_GNU_SOURCE"  # Enable GNU extensions

# Include directories
INCLUDES="-I$PROJECT_ROOT/lib/LibIsolationForest/cpp"
INCLUDES="$INCLUDES -I$PROJECT_ROOT/helper"
INCLUDES="$INCLUDES -I$PROJECT_ROOT"

# Source files
SOURCES=(
    "$PROJECT_ROOT/algorithms/Iforets_on_OPS-SAT.cpp"
    "$PROJECT_ROOT/helper/helper_functions.cpp"
    "$PROJECT_ROOT/lib/LibIsolationForest/cpp/IsolationForest.cpp"
)

# Output binary name
OUTPUT_BINARY="$BUILD_DIR/iforest_latest_arm"

echo "Compiling with the following settings:"
echo "  Compiler: $CXX"
echo "  Flags: $CXXFLAGS"
echo "  Includes: $INCLUDES"
echo "  Output: $OUTPUT_BINARY"
echo ""

# Compile the application
echo "Starting compilation..."
$CXX $CXXFLAGS $INCLUDES -o "$OUTPUT_BINARY" "${SOURCES[@]}"

if [ $? -eq 0 ]; then
    echo "✓ Compilation successful!"
    echo "  Binary: $OUTPUT_BINARY"
    
    # Verify the binary
    echo ""
    echo "Binary information:"
    file "$OUTPUT_BINARY"
    ls -lh "$OUTPUT_BINARY"
    
    # Copy to expected location for gem5 config
    cp "$OUTPUT_BINARY" "$PROJECT_ROOT/gem5_binaries/"
    mkdir -p "$PROJECT_ROOT/gem5_binaries"
    cp "$OUTPUT_BINARY" "$PROJECT_ROOT/gem5_binaries/iforest_latest_arm"
    echo "✓ Binary copied to gem5_binaries/"
    
    echo ""
    echo "=== Next steps ==="
    echo "1. Build the gem5 v21.2.1.0 Docker container:"
    echo "   cd $PROJECT_ROOT/simulators/Gem5_v21.2.1"
    echo "   docker build -t gem5:v21.2.1.0 ."
    echo ""
    echo "2. Run the simulation with custom ARM configuration:"
    echo "   docker run -it --rm \\"
    echo "     -v $PROJECT_ROOT:/workspace \\"
    echo "     -v $PROJECT_ROOT/simulators/Gem5_v21.2.1/results:/gem5/results \\"
    echo "     gem5:v21.2.1.0 \\"
    echo "     /gem5/build/ARM/gem5.opt \\"
    echo "     /workspace/simulators/simulator_configs/gem5/ARM/run_config.py \\"
    echo "     --config=/workspace/simulators/simulator_configs/gem5/ARM/simulation_config.json \\"
    echo "     --config-name=default"
    
else
    echo "✗ Compilation failed!"
    exit 1
fi
