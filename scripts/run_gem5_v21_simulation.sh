#!/bin/bash

# Script to run gem5 v21.2.1.0 simulation with ARM configuration
# This script assumes the binary has been compiled and the Docker image exists

PROJECT_ROOT="/home/mothnep/Desktop/IDS_evaluation_tool"

echo "=== Running gem5 v21.2.1.0 ARM Simulation ==="
echo "Project root: $PROJECT_ROOT"

# Check if the compiled binary exists
if [ ! -f "$PROJECT_ROOT/gem5_binaries/iforest_latest_arm" ]; then
    echo "Error: ARM binary not found at $PROJECT_ROOT/gem5_binaries/iforest_latest_arm"
    echo "Please run the build script first: $PROJECT_ROOT/scripts/build_for_gem5_v21.sh"
    exit 1
fi

# Check if Docker image exists
if ! docker image inspect gem5:v21.2.1.0 >/dev/null 2>&1; then
    echo "Error: gem5:v21.2.1.0 Docker image not found"
    echo "Please build it first:"
    echo "  cd $PROJECT_ROOT/simulators/Gem5_v21.2.1"
    echo "  docker build -t gem5:v21.2.1.0 ."
    exit 1
fi

# Create results directory if it doesn't exist
mkdir -p "$PROJECT_ROOT/simulators/Gem5_v21.2.1/results"

echo ""
echo "Starting gem5 simulation..."
echo "Configuration: ARM O3CPU with custom cache hierarchy"
echo "Binary: /workspace/gem5_binaries/iforest_latest_arm"
echo "Results will be saved to: $PROJECT_ROOT/simulators/Gem5_v21.2.1/results"
echo ""

# Run the simulation
docker run -it --rm \
    -v "$PROJECT_ROOT":/workspace \
    -v "$PROJECT_ROOT/simulators/Gem5_v21.2.1/results":/gem5/results \
    gem5:v21.2.1.0 \
    /gem5/build/ARM/gem5.opt \
    /workspace/simulators/simulator_configs/gem5/ARM/run_config.py \
    --config=/workspace/simulators/simulator_configs/gem5/ARM/simulation_config.json \
    --config-name=default

SIMULATION_EXIT_CODE=$?

echo ""
if [ $SIMULATION_EXIT_CODE -eq 0 ]; then
    echo "✓ Simulation completed successfully!"
    echo "Results are available in: $PROJECT_ROOT/simulators/Gem5_v21.2.1/results"
    echo ""
    echo "Key output files to check:"
    echo "  - stats.txt: Simulation statistics"
    echo "  - config.ini: Final configuration used"
    echo "  - terminal: Application output"
else
    echo "✗ Simulation failed with exit code: $SIMULATION_EXIT_CODE"
    echo "Check the output above for error details"
fi

exit $SIMULATION_EXIT_CODE
