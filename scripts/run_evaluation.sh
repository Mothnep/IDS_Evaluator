#!/bin/bash

# Set color codes for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Configuration - Fix path calculation since script is now in scripts/ folder
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOL_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"  # Go up one level to project root
ALGORITHMS_DIR="$TOOL_DIR/algorithms"
EXE_DIR="$ALGORITHMS_DIR/exe"  
RESULTS_DIR="$TOOL_DIR/algorithms/results"
ROC_DATA_DIR="$ALGORITHMS_DIR/ROC_CSV"  
COMPILER="g++"
COMPILER_FLAGS="-std=c++17 -Wall -Wextra -O2"
VENV_DIR="$TOOL_DIR/venv"

# Create directories if they don't exist (at project root level)
mkdir -p "$RESULTS_DIR" "$ROC_DATA_DIR" "$EXE_DIR"

# Setup Python virtual environment if it doesn't exist
setup_python_venv() {
    # Check if virtual environment exists
    if [ ! -d "$VENV_DIR" ]; then
        echo -e "${BLUE}Creating Python virtual environment...${NC}"
        # Create virtual environment
        python3 -m venv "$VENV_DIR"
        if [ $? -ne 0 ]; then
            echo -e "${RED}Failed to create virtual environment. Make sure python3-venv is installed:${NC}"
            echo -e "sudo apt-get update && sudo apt-get install -y python3-venv"
            return 1 # Exit with error if venv creation fails
        fi
        
        # Activate the virtual environment and install required packages
        echo -e "${GREEN}Installing required packages...${NC}"
        "$VENV_DIR/bin/pip" install pandas matplotlib
        if [ $? -ne 0 ]; then
            echo -e "${RED}Failed to install required packages${NC}"
            return 1 # Exit with error if package installation fails
        fi
    fi
    
    return 0 # Return success if venv exists or was created successfully
}

# Function to run Python script within the virtual environment
run_python_script() {
    local script_path="$1"
    shift # Remove first argument to pass the rest to the Python script
    
    if [ -d "$VENV_DIR" ]; then
        "$VENV_DIR/bin/python" "$script_path" "$@"
    else
        echo -e "${RED}Virtual environment not found, falling back to system Python${NC}"
        python3 "$script_path" "$@"
    fi
}

# Function to print section headers
print_header() {
    echo
    echo -e "${BLUE}=======================================${NC}"
    echo -e "${BLUE}   $1${NC}"
    echo -e "${BLUE}=======================================${NC}"
    echo
}

# Function for evaluating an algorithm
evaluate_algorithm() {
    local algo_file="$1"
    shift # Remove first argument
    local algo_name=$(basename "$algo_file" .cpp)
    
    print_header "Evaluating $algo_name"
    
    # Extract the output executable name - using EXE_DIR
    local executable="${EXE_DIR}/${algo_name}"
    
    # Always include helper functions
    local lib_files="$TOOL_DIR/helper/helper_functions.cpp"
    local include_paths="-I$TOOL_DIR"
    
    # Process each provided library path
    for lib_path in "$@"; do
        echo -e "${GREEN}Adding library: $lib_path${NC}"
        
        # Check if it's a directory
        if [ -d "$lib_path" ]; then
            # Find all .cpp files in this directory (excluding main.cpp to avoid conflicts)
            local cpp_files=$(find "$lib_path" -name "*.cpp" -not -name "main.cpp" -not -path "*/test*" -not -path "*/example*")
            for cpp_file in $cpp_files; do
                echo -e "  - Including source: $(basename "$cpp_file")"
                lib_files="$lib_files $cpp_file"
            done
            include_paths="$include_paths -I$lib_path"
        elif [ -f "$lib_path" ] && [[ "$lib_path" == *.cpp ]]; then
            # It's a single cpp file
            echo -e "  - Including source: $(basename "$lib_path")"
            lib_files="$lib_files $lib_path"  # Fixed: was using $cpp_file instead of $lib_path
            # Add its directory to include paths
            include_paths="$include_paths -I$(dirname "$lib_path")"
        fi
    done
    
    # Compile the algorithm with specified libraries
    echo -e "${GREEN}Compiling algorithm with dependencies...${NC}"
    echo "$COMPILER $COMPILER_FLAGS $algo_file $lib_files -o $executable $include_paths"
    $COMPILER $COMPILER_FLAGS $algo_file $lib_files -o "$executable" $include_paths
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Compilation failed!${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Compilation successful.${NC}"
    
    # Run the algorithm
    echo -e "\n${GREEN}Running algorithm...${NC}"
    cd "$ALGORITHMS_DIR"  # Change directory to ensure relative paths work
    "$executable"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Algorithm execution failed!${NC}"
        cd "$TOOL_DIR"
        return 1
    fi
    
    cd "$TOOL_DIR"
    
    # Plot the ROC curve using the existing helper script
    print_header "Generating ROC Plot"
    
    # Run the existing plot_roc_curve.py script with the virtual environment
    # Pass the results directory as an argument
    echo -e "${GREEN}Plotting ROC curve with existing script...${NC}"
    run_python_script "$TOOL_DIR/helper/plot_roc_curve.py" "$RESULTS_DIR"
    
    print_header "Evaluation Complete"
    echo -e "${GREEN}Results saved to ${RESULTS_DIR}${NC}"
    echo -e "${GREEN}ROC data saved to ${ROC_DATA_DIR}${NC}"
    
    # If there are PNG files in the results directory, show them
    roc_plots=$(find "$RESULTS_DIR" -name "*_roc.png")
    if [ -n "$roc_plots" ]; then
        echo -e "${GREEN}Generated ROC plots:${NC}"
        for plot in $roc_plots; do
            echo "  - $plot"
            if [ -n "$DISPLAY" ] && command -v xdg-open &> /dev/null; then
                xdg-open "$plot" &> /dev/null &
            fi
        done
    fi
    
    return 0
}

# Main script execution starts here

# Setup Python virtual environment first thing
setup_python_venv
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to set up Python environment. Some features may not work.${NC}"
fi

# Check if this script is being sourced or executed directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then 
    # Script is being executed directly
    
    # Check if at least one argument is provided
    if [ $# -eq 0 ]; then
        echo "Usage: $0 <algorithm_file.cpp> [--lib library_path [--lib library_path] ...]"
        echo "Example: $0 Iforets_on_OPS-SAT.cpp --lib LibIsolationForest/cpp"
        echo "         $0 Iforets_on_OPS-SAT.cpp --lib lib/LibIsolationForest/cpp"
        exit 1
    fi
    
    algorithm_file="$1"
    shift # Remove algorithm file from arguments
    
    # If only filename is provided, assume it's in the algorithms directory
    if [[ ! "$algorithm_file" == */* ]]; then
        algorithm_file="$ALGORITHMS_DIR/$algorithm_file"
    fi
    
    # Validate the algorithm file exists
    if [ ! -f "$algorithm_file" ]; then
        echo -e "${RED}Error: Algorithm file not found: $algorithm_file${NC}"
        exit 1
    fi
    
    # Parse remaining arguments for libraries
    lib_paths=()
    while [ $# -gt 0 ]; do
        if [ "$1" == "--lib" ] && [ $# -gt 1 ]; then
            # Handle path resolution with smart prefixing
            if [[ "$2" == /* ]]; then
                # Absolute path - use as is
                lib_path="$2"
            elif [[ "$2" == lib/* ]]; then
                # Path already starts with lib/ - just make it absolute
                lib_path="$TOOL_DIR/$2"
            else
                # Path doesn't start with lib/ - add lib/ automatically
                lib_path="$TOOL_DIR/lib/$2"
            fi
            lib_paths+=("$lib_path")
            shift 2 # Remove --lib and its value
        else
            echo -e "${RED}Error: Unknown argument or missing value: $1${NC}"
            exit 1
        fi
    done
    
    # Run the evaluation with libraries
    evaluate_algorithm "$algorithm_file" "${lib_paths[@]}"
fi