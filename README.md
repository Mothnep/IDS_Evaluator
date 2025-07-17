# Overview
The IDS Evaluation Tool is a comprehensive framework for developing, testing, and evaluating anomaly detection algorithms for Intrusion Detection Systems (IDS). This tool provides a standardized environment to implement, compile, and evaluate the performance of different detection algorithms on various datasets, with support for ARM cross-compilation and gem5 simulation for performance analysis.

# File Structure
The IDS Evaluation Tool follows a specific directory structure to organize algorithms, datasets, helper functions, simulators, and results. Below is an overview of the file structure:

```
IDS_Evaluator/
├── algorithms/           # Your detection algorithms (.cpp files)
│   ├── exe/              # Compiled algorithm executables
│   │   └── ARM/          # ARM cross-compiled binaries
│   ├── results/          # Algorithm evaluation results
│   └── ROC_CSV/          # ROC curve data points
├── datasets/             # Datasets 
│   ├── NSL-KDD/          # NSL-KDD network intrusion dataset
│   └── OPS-SAT-AD/       # OPS-SAT anomaly detection dataset
├── helper/               # Helper functions and utilities
│   ├── helper_functions.cpp  # Common evaluation functions
│   ├── helper_functions.h    # Function declarations
│   ├── dataset_array.cpp     # Embedded dataset (auto-generated)
│   ├── csv_to_cpp_array.py  # CSV to C++ array converter
│   └── plot_roc_curve.py     # ROC curve plotting utility
├── lib/                  # External libraries
├── scripts/              # Evaluation and compilation scripts
│   ├── compile_binary.sh     # ARM cross-compilation script
│   ├── run_evaluation.sh     # Single algorithm evaluation
│   └── run_full_evaluation.sh # Full evaluation with simulation
├── simulators/           # Simulation environment
│   ├── gem5_latest/      # gem5 simulator Docker environment
│   ├── mcpat/            # McPAT Docker environment
│   ├── parser/           # gem5 results parsing utilities
│   └── simulator_configs/    # Processor configuration files
│       └── gem5/
│           ├── ARM/      # ARM processor configurations
│           └── X86/      # X86 processor configurations
└── README.md             # This file
```

# Processor Architecture Configuration

Before implementing your algorithm, you need to configure the target processor architecture for simulation. The tool supports various processor configurations through JSON files located in `simulators/simulator_configs/gem5/ARM/simulation_config.json`.

## Available Configurations

The tool includes pre-configured processor architectures:

### ARM Configurations (`simulators/simulator_configs/gem5/ARM/simulation_config.json`)
- **default**: Basic ARM configuration with 4GHz clock, 512MB DDR3 memory, simple cache hierarchy
- **cortex_a72**: ARM Cortex-A72 inspired configuration with 1.8GHz clock, 2GB DDR4 memory, realistic cache sizes

## Selecting a Configuration

To use a specific configuration:
1. Navigate to the appropriate architecture folder (ARM or X86)
2. Edit the `simulation_config.json` file
3. Choose the desired configuration by name (e.g., "default" or "cortex_a72")
4. The binary path will be automatically set during compilation

## Creating Custom Configurations

You can create custom processor configurations by adding new entries to the `simulation_config.json` file. Follow this template:

```json
{
  "configurations": {
    "your_custom_config": {
      "system": {
        "mem_mode": "timing",
        "clock_domain": {
          "clock": "2.5GHz",
          "voltage_domain": {
            "voltage": "1.2V"
          }
        }
      },
      "cpu": {
        "type": "O3CPU",
        "num_cores": 1,
        "branch_prediction": {
          "enabled": true,
          "predictor_type": "tournament"
        }
      },
      "memory": {
        "override_gem5_data": false,
        "dram": {
          "type": "DDR4_2400_8x8",
          "ranks_per_channel": 1,
          "channels": 1,
          "size": "1GB"
        }
      },
      "cache_hierarchy": {
        "l1i": {
          "size": "32kB",
          "assoc": 2
        },
        "l1d": {
          "size": "32kB",
          "assoc": 2
        },
        "l2": {
          "size": "512kB",
          "assoc": 8,
          "tag_latency": 10,
          "data_latency": 10,
          "response_latency": 10,
          "mshrs": 12,
          "tgts_per_mshr": 8
        }
      },
      "process": {
        "binary": "/algorithm_binaries/your_algorithm_arm"
      }
    }
  }
}
```

### Configuration Parameters:
- **system.clock_domain.clock**: CPU clock frequency
- **system.clock_domain.voltage_domain.voltage**: Operating voltage
- **cpu.type**: CPU model (O3CPU for out-of-order, MinorCPU for in-order)
- **cpu.branch_prediction**: Branch predictor settings
- **memory.dram**: Memory type and configuration
- **cache_hierarchy**: L1 and L2 cache specifications (size, associativity, latencies)

# Dataset Integration

The tool supports embedded datasets that are compiled directly into your algorithm binary. This approach eliminates the need for file I/O during execution and ensures consistent performance measurements.

## Available Datasets

- **NSL-KDD**: Network intrusion detection dataset
- **OPS-SAT-AD**: Satellite anomaly detection dataset

## Using Embedded Datasets

When implementing your algorithm, use the `readEmbeddedDataset()` function instead of traditional file reading:

```cpp
// Load embedded dataset
vector<vector<string>> data = readEmbeddedDataset(true);  // true for header row
```

The dataset is automatically converted from CSV to C++ arrays using the `csv_to_cpp_array.py` script during compilation.

# Algorithm Implementation

To implement your detection algorithm, create a new `.cpp` file in the `algorithms/` directory. Your implementation should utilize the provided helper functions for consistent evaluation.

## Basic Algorithm Template

```cpp
// filepath: algorithms/YourAlgorithm_on_Dataset.cpp
#include "../helper/helper_functions.h"
// Include any required libraries from lib/ directory

using namespace std;

int main() {
    // Load embedded dataset
    vector<vector<string>> data = readEmbeddedDataset(true);
    
    // Prepare data structures
    vector<double> scores;      // Your anomaly scores (higher = more anomalous)
    vector<bool> labels;        // Ground truth labels (true = anomaly, false = normal)
    
    // Extract features and labels from dataset
    for (size_t i = 0; i < data.size(); i++) {
        // Extract features from data[i][0], data[i][1], etc.
        // Extract label from data[i][anomaly_column_index]
        bool isAnomaly = (data[i][1] == "1");  // Assuming column 1 contains labels
        labels.push_back(isAnomaly);
        
        // Your algorithm implementation here
        // Calculate anomaly score for this sample
        double anomalyScore = yourAlgorithmFunction(data[i]);
        scores.push_back(anomalyScore);
    }
    
    // Optional: Print basic dataset information
    printBasicInfo(data, scores, labels, 1, "YourAlgorithm");
    
    // Evaluate algorithm performance and generate ROC curve
    auto results = evaluateAlgorithm(scores, labels, "YourAlgorithm", "YourDataset");
    
    // Optional: Print detailed evaluation results
    printEvaluationResults(results, true);
    
    return 0;
}
```

## Using External Libraries

If your algorithm requires external libraries from the `lib/` directory:

1. Include the necessary headers:
```cpp
#include "../lib/dlib-19.24/dlib/svm.h"
#include "../lib/isotree/isotree.hpp"
```

2. When compiling, specify the library paths using the `--lib` flag (see compilation sections below)

## Available Helper Functions

The `helper_functions.cpp` provides several utility functions:

### Core Evaluation Functions:
- `readEmbeddedDataset()`: Load embedded dataset
- `evaluateAlgorithm()`: Complete evaluation with ROC curve generation
- `saveRocDataToCSV()`: Generate ROC curve data points
- `calculateConfusionMatrix()`: Compute confusion matrix
- `calculateMetrics()`: Calculate precision, recall, F1-score, etc.

### Information and Display Functions:
- `printBasicInfo()`: Display dataset and algorithm statistics
- `printEvaluationResults()`: Format and display evaluation metrics

### Usage Example:
```cpp
// Basic evaluation
auto results = evaluateAlgorithm(scores, labels, "MyAlgorithm", "NSL-KDD");

// Custom threshold evaluation
auto results = evaluateAlgorithm(scores, labels, "MyAlgorithm", "NSL-KDD", 0.5);

// Manual ROC curve generation
double auc = saveRocDataToCSV(scores, labels, "MyAlgorithm", "NSL-KDD");
```

# Running Algorithm Evaluation

The IDS Evaluator provides multiple ways to evaluate your algorithms depending on your needs:

## Quick Algorithm Evaluation (`run_evaluation.sh`)

For rapid testing and algorithm-specific results without processor simulation:

```bash
./scripts/run_evaluation.sh YourAlgorithm_on_Dataset.cpp
```

### Script Usage Examples:

**Basic evaluation:**
```bash
./scripts/run_evaluation.sh your_algorithm.cpp
```

**With single library:**
```bash
./scripts/run_evaluation.sh your_algorithm.cpp --lib lib/dlib-19.24
```

**With multiple libraries:**
```bash
./scripts/run_evaluation.sh your_algorithm.cpp --lib lib/dlib-19.24 --lib lib/isotree
```

**Using specific library files:**
```bash
./scripts/run_evaluation.sh your_algorithm.cpp --lib lib/isotree/isotree.cpp
```

### What This Script Does:
1. Converts CSV dataset to embedded C++ arrays
2. Compiles your algorithm with specified libraries
3. Executes the algorithm
4. Generates ROC curves and evaluation metrics
5. Saves results to `algorithms/results/` and `algorithms/ROC_CSV/`

## Full Evaluation with Simulation (`run_full_evaluation.sh`)

For comprehensive evaluation including processor simulation, power analysis, and performance metrics:

```bash
./scripts/run_full_evaluation.sh YourAlgorithm_on_Dataset.cpp
```

### Full Evaluation Process:
1. **Dataset Preparation**: Converts CSV to embedded C++ arrays
2. **X86 Compilation**: Compiles and runs algorithm for baseline results
3. **ARM Cross-compilation**: Creates ARM binary for simulation
4. **gem5 Simulation**: Runs algorithm in gem5 simulator with configured processor
5. **McPAT Analysis**: Analyzes power consumption and hardware metrics
6. **Results Generation**: Creates comprehensive performance reports

### Full Evaluation Outputs:
- Algorithm performance metrics (ROC curves, AUC, precision, recall)
- Processor simulation results (cycles, instructions, cache performance)
- Power consumption analysis
- Hardware resource utilization reports
- Comparative performance charts

## ARM Cross-Compilation Testing (`compile_binary.sh`)

To test if your algorithm and dependencies can be successfully compiled for ARM architecture:

```bash
./scripts/compile_binary.sh
```

### What This Script Does:
1. Converts dataset to embedded C++ arrays
2. Attempts ARM cross-compilation with static linking
3. Verifies binary compatibility
4. Reports compilation success/failure
5. Shows binary information (architecture, size, etc.)

### ARM Compilation Features:
- **Static linking**: Ensures binary runs in simulation environment
- **ARM optimization**: Uses appropriate compiler flags for ARM architecture
- **Dependency inclusion**: Automatically includes required libraries
- **Binary verification**: Confirms successful cross-compilation

### Troubleshooting ARM Compilation:
If compilation fails, check:
- ARM cross-compiler installation: `arm-linux-gnueabi-g++`
- Library compatibility with ARM architecture
- Static linking requirements for your dependencies

## Choosing the Right Evaluation Method

**Use `run_evaluation.sh` when:**
- Quick algorithm testing and debugging
- Generating ROC curves and basic metrics
- Iterating on algorithm development
- No need for processor-specific analysis

**Use `run_full_evaluation.sh` when:**
- Complete performance characterization needed
- Evaluating for specific hardware targets
- Power consumption analysis required
- Comprehensive reporting for research/publication

**Use `compile_binary.sh` when:**
- Testing ARM compatibility before full evaluation
- Debugging compilation issues
- Verifying cross-compilation setup
- Quick binary generation for external use

## Output Files and Results

### Algorithm Results (`algorithms/results/`):
- `AlgorithmName_DatasetName_roc.png`: ROC curve visualization
- Performance metrics and statistics

### ROC Data (`algorithms/ROC_CSV/`):
- `AlgorithmName_DatasetName_roc.csv`: ROC curve data points
- Threshold, FPR, and TPR values for analysis

### Simulation Results (Full Evaluation):
- `stats.txt`: gem5 simulation statistics
- `config.json`: Configuration used for simulation
- Power analysis reports from McPAT
- Hardware performance visualizations

## Example Workflow

1. **Develop Algorithm**: Create your algorithm in `algorithms/`
2. **Quick Test**: Use `run_evaluation.sh` for rapid iteration
3. **ARM Test**: Use `compile_binary.sh` to verify ARM compatibility
4. **Full Analysis**: Use `run_full_evaluation.sh` for comprehensive evaluation
5. **Review Results**: Analyze outputs in `algorithms/results/`

# Prerequisites and Setup

## Required Dependencies

### System Requirements:
- **Linux/Unix environment** (Ubuntu/Debian recommended)
- **GCC compiler** with C++17 support
- **Python 3.6+** with pip
- **ARM cross-compiler**: `arm-linux-gnueabi-g++`
- **Docker** (for gem5 simulation)

### Python Dependencies:
The evaluation scripts automatically set up a Python virtual environment and install:
- `pandas` (for data processing)
- `matplotlib` (for ROC curve plotting)

### ARM Cross-Compiler Installation:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi

# Verify installation
arm-linux-gnueabi-g++ --version
```

### Docker Setup (for Full Evaluation):
```bash
# Install Docker
sudo apt-get install docker.io
sudo usermod -aG docker $USER

# Build gem5 image (if not already built)
cd simulators/gem5_latest
docker build -t gem5:latest .
```

## First Time Setup

1. **Clone the repository**
2. **Install dependencies** (see above)
3. **Configure processor architecture** in `simulators/simulator_configs/gem5/`
4. **Test basic functionality**:
   ```bash
   # Test compilation
   ./scripts/compile_binary.sh
   
   # Test evaluation
   ./scripts/run_evaluation.sh algorithms/existing_algorithm.cpp
   ```

# Best Practices

## Algorithm Development:
- Use descriptive naming: `AlgorithmName_on_DatasetName.cpp`
- Include proper error handling in your algorithm
- Test with `run_evaluation.sh` before full evaluation
- Document your algorithm's parameters and assumptions

## Performance Optimization:
- Use embedded datasets to avoid I/O overhead
- Minimize memory allocations in tight loops
- Consider ARM-specific optimizations for cross-compilation
- Profile your algorithm before extensive simulation

## Result Analysis:
- Compare AUC values across different algorithms
- Analyze ROC curves for different operating points
- Review confusion matrices for threshold selection
- Consider computational complexity alongside accuracy

# Troubleshooting

## Common Issues:

**Compilation Errors:**
- Ensure all library dependencies are correctly specified
- Check C++17 compatibility of your code
- Verify ARM cross-compiler installation

**Runtime Errors:**
- Check dataset format compatibility
- Ensure sufficient memory for your algorithm
- Verify label column index in dataset

**Simulation Issues:**
- Ensure Docker is properly installed and running
- Check gem5 image availability
- Verify ARM binary compatibility

**Permission Errors:**
- Ensure scripts have execute permissions: `chmod +x scripts/*.sh`
- Check Docker group membership for simulation

For additional help, refer to the individual script documentation or check the project's issue tracker.

