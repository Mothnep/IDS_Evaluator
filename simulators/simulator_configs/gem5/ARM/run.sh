#!/bin/bash

# Source the virtual environment
source ~/Desktop/IDS_evaluation_tool/venv/bin/activate

# Run gem5 with warning filters
~/Desktop/IDS_evaluation_tool/simulators/gem5/build/ARM/gem5.opt ~/Desktop/IDS_evaluation_tool/simulators/simulator_configs/gem5/ARM/prog.py "$@" 2>&1 | grep -Ev "(CP14 unimplemented|No dot file generated|will be cast to base 2)"

# Deactivate venv when done
deactivate