#!/bin/bash

echo "=== ARM Cross-Compilation for Isolation Forest with Embedded Dataset ==="

# Define paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DATASET_CSV="$PROJECT_ROOT/datasets/OPS-SAT-AD/data/dataset.csv"
DATASET_ARRAY_CPP="$PROJECT_ROOT/helper/dataset_array.cpp"
CSV_TO_CPP_SCRIPT="$PROJECT_ROOT/helper/csv_to_cpp_array.py"

# Define source files (relative to project root)
MAIN_SOURCE="algorithms/Iforets_on_OPS-SAT.cpp"
HELPER_SOURCE="helper/helper_functions.cpp"
IFOREST_SOURCE="lib/LibIsolationForest/cpp/IsolationForest.cpp"
OUTPUT_BINARY="algorithms/exe/iforest_arm"

# Include directories
INCLUDE_DIRS="-I. -Ilib/LibIsolationForest/cpp -Ihelper"

# Change to project root directory
cd "$PROJECT_ROOT" || exit 1

echo "Current directory: $(pwd)"

mkdir -p "algorithms/exe"

# Step 1: Check if dataset CSV exists
if [ ! -f "$DATASET_CSV" ]; then
    echo "✗ Error: Dataset CSV not found at $DATASET_CSV"
    exit 1
fi

echo "✓ Found dataset CSV: $DATASET_CSV"

# Step 2: Check if Python script exists
if [ ! -f "$CSV_TO_CPP_SCRIPT" ]; then
    echo "✗ Error: CSV to C++ conversion script not found at $CSV_TO_CPP_SCRIPT"
    exit 1
fi

echo "✓ Found conversion script: $CSV_TO_CPP_SCRIPT"

# Step 3: Convert CSV to C++ array
echo "Converting CSV dataset to C++ array..."
python3 "$CSV_TO_CPP_SCRIPT" "$DATASET_CSV" "$DATASET_ARRAY_CPP"

if [ $? -ne 0 ]; then
    echo "✗ Error: Failed to convert CSV to C++ array"
    exit 1
fi

echo "✓ CSV conversion successful: $DATASET_ARRAY_CPP"

# Step 4: Verify that the C++ array file was created
if [ ! -f "$DATASET_ARRAY_CPP" ]; then
    echo "✗ Error: Generated C++ array file not found"
    exit 1
fi

echo "✓ Generated C++ array file verified"

# Step 5: Check if all source files exist
for source_file in "$MAIN_SOURCE" "$HELPER_SOURCE" "$IFOREST_SOURCE"; do
    if [ ! -f "$source_file" ]; then
        echo "✗ Error: Source file not found: $source_file"
        exit 1
    fi
done

echo "✓ All source files found"

# Step 6: Compile with ARM cross-compiler
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
    echo ""
    echo "Binary information:"
    file $OUTPUT_BINARY
    ls -lh $OUTPUT_BINARY
    echo ""
    echo "Generated files:"
    echo "  - Dataset array: $DATASET_ARRAY_CPP"
    echo "  - ARM binary: $OUTPUT_BINARY"
else
    echo "✗ ARM compilation failed"
    exit 1
fi

echo ""
echo "=== Compilation Complete ==="