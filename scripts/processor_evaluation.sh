#!/bin/bash

echo "=== ARM Cross-Compilation for gem5 Simulation ==="

# Compile a simple test to verify gem5 compatibility
echo "Compiling simple test for ARM (gem5 compatible)..."
arm-linux-gnueabi-g++ -std=c++11 -Wall -O2 -static \
    -march=armv7-a -mfloat-abi=soft \
    -fno-stack-protector -fno-pie -no-pie \
    -o test_gem5_arm test_gem5.cpp

if [ $? -eq 0 ]; then
    echo "✓ ARM test compilation successful: test_gem5_arm"
    echo "Binary architecture:"
    file test_gem5_arm
    echo ""
    echo "Binary size:"
    ls -lh test_gem5_arm
    echo ""
    echo "✓ Ready for gem5 testing!"
    echo "Binary location: $(pwd)/test_gem5_arm"
else
    echo "✗ ARM test compilation failed"
    exit 1
fi