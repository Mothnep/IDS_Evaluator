# Overview
The IDS Evaluation Tool is a comprehensive framework for developing, testing, and evaluating anomaly detection algorithms for Intrusion Detection Systems (IDS). This tool provides a standardized environment to implement, compile, and evaluate the performance of different detection algorithms on various datasets.

# File Structure
The IDS Evaluation Tool follows a specific directory structure to organize algorithms, datasets, helper functions, and results. Below is an overview of the file structure:

```
IDS_evaluation_tool/
├── algorithms/           # Your detection algorithms (.cpp files)
│   ├── exe/              # Compiled algorithm executables
│   ├── models/           # Trained models and/or parameters (if applicable)
│   └── ROC_CSV/          # ROC curve data points
├── datasets/             # Test datasets 
│   └── OPS-SAT-AD/       # OPS-SAT anomaly detection dataset (example)
├── helper/               # Helper functions and utilities
│   ├── helper_functions.cpp  # Common evaluation functions
│   └── plot_roc_curve.py     # ROC curve plotting utility
├── lib/                  # External libraries
│   └── LibIsolationForest/   # Isolation Forest implementation (example)
├── results/              # Evaluation results and ROC plots
├── run_evaluation.sh     # Main evaluation script
└── README.md             # This file
```

# Algorithm Implementation Example
To implement your algorithm, create a new `.cpp` file in the `algorithms/` directory. Below is a template for your algorithm implementation:

```cpp
// filepath: algorithms/YourAlgorithm_on_Dataset.cpp
#include "../helper/helper_functions.h"
// Include any other required libraries

using namespace std;

int main() {
    // Load and prepare data
    vector<vector<string>> data = readCSV(true, "../datasets/yourDataset.csv");
    
    // Process data
    vector<double> scores;  // Your anomaly scores
    vector<bool> isAnomaly; // Ground truth labels
    
    // Your algorithm implementation here
    
    // Evaluate algorithm performance
    auto results = evaluateAlgorithm(scores, isAnomaly, "YourAlgorithm", "YourDataset");
    
    return 0;
}
```

# Running the Evaluation
To run the evaluation of your algorithm on the dataset, execute the `run_evaluation.sh` script after placing your algorithm and dataset in the appropriate directories. This script will compile your algorithm, run the evaluation, and generate the results and ROC plots.

## Script usage
The `run_evaluation.sh` script can take multiple arguments. Here are a few examples

- Basic run
`./run_evalusation.sh yourAlgorithmFileName.cpp`

- Run with library (can either specify folder name or .cpp file)
`./run_evalusation.sh yourAlgorithmFileName.cpp --lib yourLibrary.cpp`

- Run with multiple libraries (specify each one or put them under the same folder)
`./run_evalusation.sh yourAlgorithmFileName.cpp --lib yourFirstLibrary.cpp --lib yourSecondLibrary .cpp`
`./run_evalusation.sh yourAlgorithmFileName.cpp --lib yourCommonLibraryFolder`

