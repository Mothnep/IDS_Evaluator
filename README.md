# Overview
The IDS Evaluation Tool is a comprehensive framework for developing, testing, and evaluating anomaly detection algorithms for Intrusion Detection Systems (IDS). This tool provides a standardized environment to implement, compile, evaluate the performance of different detection algorithms on various datasets and benchmark processor architecture performance for the given algorithm.

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
│   ├── helper_functions.cpp  # Common evaluation and helper functions
│   ├── helper_functions.h   
│   ├── dataset_array.cpp     # Embedded dataset (auto-generated)
│   ├── csv_to_cpp_array.py  # CSV to C++ array converter
│   └── plot_roc_curve.py     # ROC curve plotting utility
├── lib/                  # External coding libraries
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
1. Navigate to the appropriate architecture folder
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

## Configuration Parameters:
All the tweakable parameters can be found in the README in the 'Simulator_configs' folder.

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

The dataset is automatically converted from CSV to C++ arrays using the `csv_to_cpp_array.py` script during compilation. You DO NOT NEED to provide the name of the dataset file in the code. This will be requested when running the automation script.

# Algorithm Implementation

To implement your detection algorithm, create a new `.cpp` file in the `algorithms/` directory. Your implementation should utilize the provided helper functions for consistent evaluation.

## Basic Algorithm Template
An example implementation can be found under `lightweight_lof_on_OPS-SAT.cpp`. In order to run the whole evaluation you will NEED to use the "readEmbeddedDataset" and the "evaluateAlgorithm" functions. A basic structure looks something like this:

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



