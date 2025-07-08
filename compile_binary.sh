#!/bin/bash

echo "=== ARM Cross-Compilation for Isolation Forest with Embedded Dataset ==="

# Define source files
MAIN_SOURCE="algorithms/Iforets_on_OPS-SAT.cpp"
HELPER_SOURCE="helper/helper_functions.cpp"
IFOREST_SOURCE="lib/LibIsolationForest/cpp/IsolationForest.cpp"
OUTPUT_BINARY="iforest_embedded_arm"

# Include directories
INCLUDE_DIRS="-I. -Ilib/LibIsolationForest/cpp -Ihelper"

echo "Compiling Isolation Forest with embedded dataset..."

arm-linux-gnueabi-g++ -std=c++17 -Wall -O2 -static \
    -march=armv7-a -mfloat-abi=soft \
    -fno-stack-protector -fno-pie -no-pie \
    $INCLUDE_DIRS \
    -o $OUTPUT_BINARY \
    $MAIN_SOURCE \
    $HELPER_SOURCE \
    $IFOREST_SOURCE

if [ $? -eq 0 ]; then
    echo "✓ ARM compilation successful: $OUTPUT_BINARY"
    file $OUTPUT_BINARY
    ls -lh $OUTPUT_BINARY
else
    echo "✗ ARM compilation failed"
    exit 1
fi