import csv
import sys
import os

def csv_to_cpp_array(csv_file_path, output_file_path=None, max_rows=2000):  # Add max_rows parameter
    """
    Convert CSV file to C++ array format
    """
    
    if not os.path.exists(csv_file_path):
        print(f"Error: CSV file {csv_file_path} not found!")
        return False
    
    # If no output file specified, create one based on input name
    if output_file_path is None:
        base_name = os.path.splitext(os.path.basename(csv_file_path))[0]
        output_file_path = os.path.join(os.path.dirname(__file__), f"{base_name}_array.cpp")
    
    try:
        with open(csv_file_path, 'r') as csvfile:
            csv_reader = csv.reader(csvfile)
            
            # Read header
            header = next(csv_reader)
            
            # Read all data rows but limit them
            data_rows = []
            row_count = 0
            for row in csv_reader:
                if len(row) >= len(header):  # Ensure row has enough columns
                    data_rows.append(row)
                    row_count += 1
                    if max_rows and row_count >= max_rows:  # Stop after max_rows
                        break
            
            print(f"Read {len(data_rows)} data rows with {len(header)} columns (limited to {max_rows})")
            
            # Generate C++ code
            cpp_content = generate_cpp_array_code(header, data_rows)
            
            # Write to output file
            with open(output_file_path, 'w') as outfile:
                outfile.write(cpp_content)
            
            print(f"C++ array file generated: {output_file_path}")
            return True
            
    except Exception as e:
        print(f"Error processing CSV file: {e}")
        return False

def generate_cpp_array_code(header, data_rows):
    """
    Generate C++ code with data arrays
    """
    
    cpp_code = """// Auto-generated C++ arrays from CSV data
#include <vector>
#include <string>

namespace DataArrays {

"""
    
    # Add header array
    cpp_code += "    // Column headers\n"
    cpp_code += "    const std::vector<std::string> headers = {\n"
    for i, col in enumerate(header):
        cpp_code += f"        \"{col}\""
        if i < len(header) - 1:
            cpp_code += ","
        cpp_code += "\n"
    cpp_code += "    };\n\n"
    
    # Add dimensions
    cpp_code += f"    const int NUM_ROWS = {len(data_rows)};\n"
    cpp_code += f"    const int NUM_COLS = {len(header)};\n\n"
    
    # Add 2D data array
    cpp_code += "    // Data matrix (row-major order)\n"
    cpp_code += "    const std::vector<std::vector<std::string>> data = {\n"
    
    for i, row in enumerate(data_rows):
        cpp_code += "        {"
        for j, cell in enumerate(row):
            # Escape quotes in cell data
            escaped_cell = cell.replace('"', '\\"')
            cpp_code += f'"{escaped_cell}"'
            if j < len(row) - 1:
                cpp_code += ", "
        cpp_code += "}"
        if i < len(data_rows) - 1:
            cpp_code += ","
        cpp_code += "\n"
    
    cpp_code += "    };\n\n"
    
    # Add helper functions
    cpp_code += """    // Helper functions
    std::string getCell(int row, int col) {
        if (row >= 0 && row < NUM_ROWS && col >= 0 && col < NUM_COLS) {
            return data[row][col];
        }
        return "";
    }
    
    std::string getCell(int row, const std::string& column_name) {
        for (int i = 0; i < headers.size(); i++) {
            if (headers[i] == column_name) {
                return getCell(row, i);
            }
        }
        return "";
    }
    
    double getCellAsDouble(int row, int col) {
        std::string cell = getCell(row, col);
        if (!cell.empty()) {
            try {
                return std::stod(cell);
            } catch (...) {
                return 0.0;
            }
        }
        return 0.0;
    }
    
    double getCellAsDouble(int row, const std::string& column_name) {
        std::string cell = getCell(row, column_name);
        if (!cell.empty()) {
            try {
                return std::stod(cell);
            } catch (...) {
                return 0.0;
            }
        }
        return 0.0;
    }
    
    int getCellAsInt(int row, int col) {
        std::string cell = getCell(row, col);
        if (!cell.empty()) {
            try {
                return std::stoi(cell);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    }
    
    int getCellAsInt(int row, const std::string& column_name) {
        std::string cell = getCell(row, column_name);
        if (!cell.empty()) {
            try {
                return std::stoi(cell);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    }
    
    std::vector<std::string> getRow(int row) {
        if (row >= 0 && row < NUM_ROWS) {
            return data[row];
        }
        return std::vector<std::string>();
    }
    
    std::vector<std::string> getColumn(int col) {
        std::vector<std::string> column;
        if (col >= 0 && col < NUM_COLS) {
            for (const auto& row : data) {
                if (col < row.size()) {
                    column.push_back(row[col]);
                }
            }
        }
        return column;
    }
    
    std::vector<std::string> getColumn(const std::string& column_name) {
        for (int i = 0; i < headers.size(); i++) {
            if (headers[i] == column_name) {
                return getColumn(i);
            }
        }
        return std::vector<std::string>();
    }

} // namespace DataArrays
"""
    
    return cpp_code

def main():
    if len(sys.argv) < 2:
        print("Usage: python csv_to_cpp_array.py <csv_file> [output_file]")
        print("Example: python csv_to_cpp_array.py ../datasets/OPS-SAT-AD/data/dataset.csv")
        return
    
    csv_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    success = csv_to_cpp_array(csv_file, output_file)
    if success:
        print("Conversion completed successfully!")
    else:
        print("Conversion failed!")

if __name__ == "__main__":
    main()