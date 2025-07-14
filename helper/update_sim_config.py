#!/usr/bin/env python3
"""
Helper script to update gem5 simulation configuration with the correct binary path
"""
import json
import sys
import os
import argparse

def update_simulation_config(config_file, binary_name, config_name="default"):
    """
    Update the simulation configuration with the correct binary path
    
    Args:
        config_file: Path to simulation_config.json
        binary_name: Name of the ARM binary (without _arm suffix)
        config_name: Configuration name to update (default: "default")
    """
    try:
        # Read the current configuration
        with open(config_file, 'r') as f:
            config_data = json.load(f)
        
        # Check if the configuration exists
        if config_name not in config_data["configurations"]:
            print(f"Error: Configuration '{config_name}' not found in {config_file}")
            print(f"Available configurations: {list(config_data['configurations'].keys())}")
            return False
        
        # Update the binary path - the binary will be mounted to /algorithm_binaries in the container
        arm_binary_name = f"{binary_name}_arm"
        new_binary_path = f"/algorithm_binaries/{arm_binary_name}"
        
        # Update the specific configuration
        config_data["configurations"][config_name]["process"]["binary"] = new_binary_path
        
        # Write back the updated configuration
        with open(config_file, 'w') as f:
            json.dump(config_data, f, indent=2)
        
        print(f"✓ Updated '{config_name}' configuration with binary: {new_binary_path}")
        return True
        
    except FileNotFoundError:
        print(f"Error: Configuration file not found: {config_file}")
        return False
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in configuration file: {e}")
        return False
    except Exception as e:
        print(f"Error updating configuration: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Update gem5 simulation configuration")
    parser.add_argument("config_file", help="Path to simulation_config.json")
    parser.add_argument("binary_name", help="Name of the algorithm binary (without _arm suffix)")
    parser.add_argument("--config-name", default="default", help="Configuration name to update")
    
    args = parser.parse_args()
    
    success = update_simulation_config(args.config_file, args.binary_name, args.config_name)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
