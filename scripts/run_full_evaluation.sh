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

# Function to convert CSV dataset to C++ array
convert_dataset_to_cpp() {
    local csv_file="$1"
    local dataset_array_file="$TOOL_DIR/helper/dataset_array.cpp"
    local csv_to_cpp_script="$TOOL_DIR/helper/csv_to_cpp_array.py"
    
    print_header "Converting Dataset to C++ Array"
    
    # Check if CSV file exists
    if [ ! -f "$csv_file" ]; then
        echo -e "${RED}Error: CSV dataset file not found: $csv_file${NC}"
        return 1
    fi
    
    # Check if conversion script exists
    if [ ! -f "$csv_to_cpp_script" ]; then
        echo -e "${RED}Error: CSV to C++ conversion script not found: $csv_to_cpp_script${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Converting CSV dataset: $csv_file${NC}"
    echo -e "${GREEN}Output will be: $dataset_array_file${NC}"
    
    # Run the conversion script
    run_python_script "$csv_to_cpp_script" "$csv_file" "$dataset_array_file"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Dataset conversion failed!${NC}"
        return 1
    fi
    
    # Verify the output file was created
    if [ ! -f "$dataset_array_file" ]; then
        echo -e "${RED}Error: Dataset array file was not created: $dataset_array_file${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Dataset conversion successful!${NC}"
    return 0
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

# Function for ARM cross-compilation
compile_for_arm() {
    local algo_file="$1"
    shift # Remove first argument
    local algo_name=$(basename "$algo_file" .cpp)
    
    print_header "ARM Cross-Compilation for $algo_name"
    
    # Check if ARM cross-compiler is available
    if ! command -v arm-linux-gnueabi-g++ &> /dev/null; then
        echo -e "${RED}Warning: ARM cross-compiler (arm-linux-gnueabi-g++) not found${NC}"
        echo -e "${RED}Skipping ARM compilation. To install:${NC}"
        echo -e "${RED}  sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi${NC}"
        return 1
    fi
    
    # Create ARM output directory
    local arm_exe_dir="$ALGORITHMS_DIR/exe/ARM"
    mkdir -p "$arm_exe_dir"
    
    # Define ARM output binary
    local arm_executable="${arm_exe_dir}/${algo_name}_arm"
    
    # Always include helper functions and dataset array
    local lib_files="$TOOL_DIR/helper/helper_functions.cpp"
    local include_paths="-I$TOOL_DIR"
    
    # Process each provided library path (same logic as regular compilation)
    for lib_path in "$@"; do
        echo -e "${GREEN}Adding library for ARM compilation: $lib_path${NC}"
        
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
            lib_files="$lib_files $lib_path"
            # Add its directory to include paths
            include_paths="$include_paths -I$(dirname "$lib_path")"
        fi
    done
    
    # ARM compilation flags (based on compile_binary.sh)
    local arm_compiler="arm-linux-gnueabi-g++"
    local arm_flags="-std=c++17 -Wall -O2 -static -march=armv7-a -mfloat-abi=soft -fno-stack-protector -fno-pie -no-pie"
    
    # Compile for ARM
    echo -e "${GREEN}Compiling for ARM with dependencies...${NC}"
    echo "$arm_compiler $arm_flags $algo_file $lib_files -o $arm_executable $include_paths"
    $arm_compiler $arm_flags $algo_file $lib_files -o "$arm_executable" $include_paths
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}ARM compilation failed!${NC}"
        return 1
    fi
    
    echo -e "${GREEN}ARM compilation successful!${NC}"
    
    # Show binary information
    echo -e "\n${GREEN}ARM Binary Information:${NC}"
    if command -v file &> /dev/null; then
        file "$arm_executable"
    fi
    ls -lh "$arm_executable"
    
    echo -e "${GREEN}ARM binary saved to: $arm_executable${NC}"
    
    return 0
}

# Function for gem5 simulation
run_gem5_simulation() {
    local algo_name="$1"
    local gem5_config_name="$2"
    
    print_header "gem5 Simulation for $algo_name"
    
    # Check if Docker is available
    if ! command -v docker &> /dev/null; then
        echo -e "${RED}Warning: Docker not found. Skipping gem5 simulation.${NC}"
        echo -e "${RED}To install Docker: https://docs.docker.com/get-docker/${NC}"
        return 1
    fi
    
    # Check if gem5:latest image exists
    if ! docker image inspect gem5:latest &> /dev/null; then
        echo -e "${RED}Warning: gem5:latest Docker image not found.${NC}"
        echo -e "${RED}Please build the gem5 Docker image first:${NC}"
        echo -e "${RED}  cd $TOOL_DIR/simulators/gem5_latest${NC}"
        echo -e "${RED}  docker build -t gem5:latest .${NC}"
        return 1
    fi
    
    # Define paths
    local arm_exe_dir="$ALGORITHMS_DIR/exe/ARM"
    local sim_configs_dir="$TOOL_DIR/simulators/simulator_configs"
    local gem5_results_dir="$TOOL_DIR/simulators/gem5_latest/results"
    local sim_config_file="$sim_configs_dir/gem5/ARM/simulation_config.json"
    local update_script="$TOOL_DIR/helper/update_sim_config.py"
    
    # Check if ARM binary exists
    local arm_binary="${arm_exe_dir}/${algo_name}_arm"
    if [ ! -f "$arm_binary" ]; then
        echo -e "${RED}Error: ARM binary not found: $arm_binary${NC}"
        echo -e "${RED}Please ensure ARM compilation completed successfully.${NC}"
        return 1
    fi
    
    # Create results directory if it doesn't exist
    mkdir -p "$gem5_results_dir"
    
    # Update simulation configuration with correct binary path
    echo -e "${GREEN}Updating simulation configuration...${NC}"
    run_python_script "$update_script" "$sim_config_file" "$algo_name" --config-name "$gem5_config_name"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to update simulation configuration${NC}"
        return 1
    fi
    
    # Run gem5 simulation in Docker
    echo -e "${GREEN}Starting gem5 simulation with configuration: $gem5_config_name${NC}"
    echo -e "${GREEN}This may take several minutes...${NC}"
    
    docker run --rm -it \
        -v "$sim_configs_dir:/simulator_configs" \
        -v "$gem5_results_dir:/gem5/m5out" \
        -v "$arm_exe_dir:/algorithm_binaries" \
        gem5:latest \
        /gem5/build/ARM/gem5.opt /simulator_configs/gem5/ARM/run_config.py \
        --config /simulator_configs/gem5/ARM/simulation_config.json \
        --config-name "$gem5_config_name"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}gem5 simulation failed!${NC}"
        return 1
    fi
    
    echo -e "${GREEN}gem5 simulation completed successfully!${NC}"
    
    # Show generated output files
    if [ -d "$gem5_results_dir" ]; then
        echo -e "\n${GREEN}Generated simulation files:${NC}"
        ls -la "$gem5_results_dir"
        
        # Check for key output files
        if [ -f "$gem5_results_dir/stats.txt" ]; then
            echo -e "${GREEN}✓ Statistics file: $gem5_results_dir/stats.txt${NC}"
        fi
        if [ -f "$gem5_results_dir/config.json" ]; then
            echo -e "${GREEN}✓ Configuration file: $gem5_results_dir/config.json${NC}"
        fi
    fi
    
    return 0
}

# Function for McPAT power analysis
run_mcpat_analysis() {
    local algo_name="$1"
    
    print_header "McPAT Power Analysis for $algo_name"
    
    # Check if Docker is available
    if ! command -v docker &> /dev/null; then
        echo -e "${RED}Warning: Docker not found. Skipping McPAT analysis.${NC}"
        return 1
    fi
    
    # Check if mcpat:v1.3.0 image exists
    if ! docker image inspect mcpat:v1.3.0 &> /dev/null; then
        echo -e "${RED}Warning: mcpat:v1.3.0 Docker image not found.${NC}"
        echo -e "${RED}Please build the McPAT Docker image first:${NC}"
        echo -e "${RED}  cd $TOOL_DIR/simulators/mcpat${NC}"
        echo -e "${RED}  docker build -t mcpat:v1.3.0 .${NC}"
        return 1
    fi
    
    # Define paths
    local gem5_results_dir="$TOOL_DIR/simulators/gem5_latest/results"
    local parser_dir="$TOOL_DIR/simulators/parser"
    local parser_results_dir="$parser_dir/results"
    local config_file="$gem5_results_dir/config.json"
    local stats_file="$gem5_results_dir/stats.txt"
    local template_file="$parser_dir/templates/ARM_O3.xml"
    local mcpat_input_file="$parser_results_dir/mcpat-in.xml"
    local parser_script="$parser_dir/parser.py"
    
    # Check if gem5 results exist
    if [ ! -f "$config_file" ]; then
        echo -e "${RED}Error: gem5 config.json not found: $config_file${NC}"
        echo -e "${RED}Please ensure gem5 simulation completed successfully.${NC}"
        return 1
    fi
    
    if [ ! -f "$stats_file" ]; then
        echo -e "${RED}Error: gem5 stats.txt not found: $stats_file${NC}"
        echo -e "${RED}Please ensure gem5 simulation completed successfully.${NC}"
        return 1
    fi
    
    # Check if parser files exist
    if [ ! -f "$parser_script" ]; then
        echo -e "${RED}Error: Parser script not found: $parser_script${NC}"
        return 1
    fi
    
    if [ ! -f "$template_file" ]; then
        echo -e "${RED}Error: McPAT template not found: $template_file${NC}"
        return 1
    fi
    
    # Create parser results directory
    mkdir -p "$parser_results_dir"
    
    # Run the parser to generate McPAT input
    print_header "Parsing gem5 Results"
    echo -e "${GREEN}Running parser to convert gem5 results to McPAT format...${NC}"
    
    cd "$parser_dir"
    run_python_script "$parser_script" \
        -c "$config_file" \
        -s "$stats_file" \
        -t "$template_file" \
        -o "$mcpat_input_file" \
        --verbose
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Parser failed to generate McPAT input!${NC}"
        cd "$TOOL_DIR"
        return 1
    fi
    
    # Check if McPAT input was generated
    if [ ! -f "$mcpat_input_file" ]; then
        echo -e "${RED}Error: McPAT input file was not generated: $mcpat_input_file${NC}"
        cd "$TOOL_DIR"
        return 1
    fi
    
    echo -e "${GREEN}McPAT input file generated successfully: $mcpat_input_file${NC}"
    
    # Run McPAT analysis
    print_header "Running McPAT Power Analysis"
    echo -e "${GREEN}Running McPAT power and area analysis...${NC}"
    
    docker run --rm \
        -v "$parser_results_dir:/data" \
        mcpat:v1.3.0 \
        ./mcpat -infile /data/mcpat-in.xml -print_level 2
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}McPAT analysis failed!${NC}"
        cd "$TOOL_DIR"
        return 1
    fi
    
    echo -e "${GREEN}McPAT power analysis completed successfully!${NC}"
    
    # Copy McPAT input to results directory for reference
    local algorithm_results_mcpat="$RESULTS_DIR/mcpat-${algo_name}.xml"
    cp "$mcpat_input_file" "$algorithm_results_mcpat"
    echo -e "${GREEN}McPAT input file copied to: $algorithm_results_mcpat${NC}"
    
    cd "$TOOL_DIR"
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
        echo "Usage: $0 <algorithm_file.cpp> [--data dataset.csv] [--lib library_path [--lib library_path] ...] [--config gem5_config_name]"
        echo "Examples:"
        echo "         $0 enhanced_lof_on_OPS-SAT.cpp --data OPS-SAT-AD/data/dataset.csv"
        echo "         $0 enhanced_lof_on_OPS-SAT.cpp --data OPS-SAT-AD/data/dataset.csv --config cortex_a72"
        echo "         $0 Iforets_on_OPS-SAT.cpp --data OPS-SAT-AD/data/dataset.csv --lib LibIsolationForest/cpp"
        echo "         $0 Iforets_on_OPS-SAT.cpp --lib lib/LibIsolationForest/cpp --config default"
        echo ""
        echo "Available gem5 configurations: default, cortex_a72"
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
    
    # Parse remaining arguments for dataset, libraries, and gem5 config
    lib_paths=()
    dataset_file=""
    gem5_config_name="default"  # Default gem5 configuration
    
    while [ $# -gt 0 ]; do
        if [ "$1" == "--data" ] && [ $# -gt 1 ]; then
            # Handle dataset file path resolution
            if [[ "$2" == /* ]]; then
                # Absolute path - use as is
                dataset_file="$2"
            else
                # Relative path - resolve from datasets directory
                dataset_file="$TOOL_DIR/datasets/$2"
            fi
            shift 2 # Remove --data and its value
        elif [ "$1" == "--lib" ] && [ $# -gt 1 ]; then
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
        elif [ "$1" == "--config" ] && [ $# -gt 1 ]; then
            gem5_config_name="$2"
            shift 2 # Remove --config and its value
        else
            echo -e "${RED}Error: Unknown argument or missing value: $1${NC}"
            exit 1
        fi
    done
    
    # Convert dataset to C++ array if dataset file is provided
    if [ -n "$dataset_file" ]; then
        convert_dataset_to_cpp "$dataset_file"
        if [ $? -ne 0 ]; then
            echo -e "${RED}Failed to convert dataset. Exiting.${NC}"
            exit 1
        fi
    else
        echo -e "${BLUE}No dataset file provided. Using existing dataset_array.cpp${NC}"
        # Check if dataset_array.cpp exists
        if [ ! -f "$TOOL_DIR/helper/dataset_array.cpp" ]; then
            echo -e "${RED}Warning: No dataset_array.cpp found and no dataset provided with --data${NC}"
            echo -e "${RED}The algorithm may fail if it tries to use readEmbeddedDataset()${NC}"
        fi
    fi
    
    # Run the evaluation with libraries
    evaluate_algorithm "$algorithm_file" "${lib_paths[@]}"
    
    # After successful evaluation, compile for ARM
    if [ $? -eq 0 ]; then
        echo -e "\n${BLUE}Evaluation completed successfully. Proceeding with ARM compilation...${NC}"
        compile_for_arm "$algorithm_file" "${lib_paths[@]}"
        
        if [ $? -eq 0 ]; then
            echo -e "\n${BLUE}ARM compilation completed successfully. Proceeding with gem5 simulation...${NC}"
            
            # Extract algorithm name for gem5 simulation
            algo_name=$(basename "$algorithm_file" .cpp)
            run_gem5_simulation "$algo_name" "$gem5_config_name"
            
            if [ $? -eq 0 ]; then
                echo -e "\n${BLUE}gem5 simulation completed successfully. Proceeding with McPAT power analysis...${NC}"
                run_mcpat_analysis "$algo_name"
                
                if [ $? -eq 0 ]; then
                    print_header "Full Evaluation Pipeline Complete"
                    echo -e "${GREEN}✓ Dataset conversion completed${NC}"
                    echo -e "${GREEN}✓ Algorithm evaluation completed${NC}"
                    echo -e "${GREEN}✓ ARM cross-compilation completed${NC}"
                    echo -e "${GREEN}✓ gem5 simulation completed${NC}"
                    echo -e "${GREEN}✓ McPAT power analysis completed${NC}"
                    echo -e "${GREEN}✓ All results and binaries saved${NC}"
                    echo -e "\n${GREEN}Results locations:${NC}"
                    echo -e "  - Algorithm results: $RESULTS_DIR${NC}"
                    echo -e "  - ROC data: $ROC_DATA_DIR${NC}"
                    echo -e "  - ARM binary: $ALGORITHMS_DIR/exe/ARM/${algo_name}_arm${NC}"
                    echo -e "  - gem5 results: $TOOL_DIR/simulators/gem5_latest/results${NC}"
                    echo -e "  - McPAT input: $RESULTS_DIR/mcpat-${algo_name}.xml${NC}"
                else
                    echo -e "${RED}McPAT analysis failed, but gem5 simulation was successful${NC}"
                    print_header "Partial Pipeline Complete"
                    echo -e "${GREEN}✓ Dataset conversion completed${NC}"
                    echo -e "${GREEN}✓ Algorithm evaluation completed${NC}"
                    echo -e "${GREEN}✓ ARM cross-compilation completed${NC}"
                    echo -e "${GREEN}✓ gem5 simulation completed${NC}"
                    echo -e "${RED}✗ McPAT power analysis failed${NC}"
                fi
            else
                echo -e "${RED}gem5 simulation failed, but ARM compilation was successful${NC}"
            fi
        else
            echo -e "${RED}ARM compilation failed, but evaluation was successful${NC}"
        fi
    else
        echo -e "${RED}Evaluation failed. Skipping ARM compilation and gem5 simulation.${NC}"
        exit 1
    fi
fi