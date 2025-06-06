import m5
from m5.objects import *
from m5.util import addToPath
import argparse
import json
import os
import sys

# Add gem5 configs to path
addToPath('/home/mothnep/Desktop/IDS_evaluation_tool/simulators/gem5/configs')

def load_config(config_file, config_name="default"):
    """Load configuration from JSON file
    
    Args:
        config_file: Path to the JSON configuration file
        config_name: Name of the configuration to load (default: "default")
    
    Returns:
        The requested configuration dictionary
    
    Raises:
        KeyError: If the requested configuration doesn't exist
    """
    with open(config_file, 'r') as f:
        config = json.load(f)
    
    if config_name not in config["configurations"]:
        available_configs = list(config["configurations"].keys())
        raise KeyError(f"Configuration '{config_name}' not found. Available configurations: {available_configs}")
    
    return config["configurations"][config_name]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", type=str, default="simulation_config.json",
                        help="Configuration file")
    parser.add_argument("--config-name", type=str, default="default",
                        help="Name of the configuration to use")
    args = parser.parse_args()
    
    # Load configuration
    config_path = os.path.join(os.path.dirname(__file__), args.config)
    config = load_config(config_path, args.config_name)
    print(f"Loaded configuration: {config_path}, using profile: {args.config_name}")
    
    # Create the system
    system = System()
    
    # Clock domain settings
    system.clk_domain = SrcClockDomain()
    system.clk_domain.clock = config["system"]["clock_domain"]["clock"]

    # Voltage domain settings 
    system.clk_domain.voltage_domain = VoltageDomain()
    system.clk_domain.voltage_domain.voltage = config["system"]["clock_domain"]["voltage_domain"]["voltage"]
    
    # Memory mode
    system.mem_mode = config["system"]["mem_mode"]
    
    # Memory ranges
    system.mem_ranges = [AddrRange(config["memory"]["size"])]
    
    # Create CPU
    cpu_config = config["cpu"]
    cpu_type = cpu_config["type"]
    
    # Select CPU class based on type
    if cpu_type == "TimingSimpleCPU":
        system.cpu = ArmTimingSimpleCPU()
    elif cpu_type == "AtomicSimpleCPU":
        system.cpu = ArmAtomicSimpleCPU()
    elif cpu_type == "DerivO3CPU" or cpu_type == "O3CPU":
        system.cpu = ArmO3CPU()
    elif cpu_type == "MinorCPU":
        system.cpu = ArmMinorCPU()
    else:
        print(f"Unknown CPU type: {cpu_type}, defaulting to ArmTimingSimpleCPU")
        system.cpu = ArmTimingSimpleCPU()
    
    # System-wide memory bus
    system.membus = SystemXBar()
    
    # Connect CPU ports to memory bus
    system.cpu.icache_port = system.membus.cpu_side_ports
    system.cpu.dcache_port = system.membus.cpu_side_ports
    
    # Set up interrupt controller
    system.cpu.createInterruptController()
    system.system_port = system.membus.cpu_side_ports
    
    
    # Create memory controller
    system.mem_ctrl = MemCtrl()
    
    # Configure DRAM using specified type
    dram_type = config["memory"]["dram"]["type"]
    
    # Map the DRAM type from the config to the gem5 class
    if dram_type == "DDR4_2400_8x8":
        system.mem_ctrl.dram = DDR4_2400_8x8()
    elif dram_type == "DDR3_1600_8x8":
        system.mem_ctrl.dram = DDR3_1600_8x8()
    elif dram_type == "DDR4_3200_8x8":
        system.mem_ctrl.dram = DDR4_3200_8x8()
    elif dram_type == "LPDDR3_1600_x32":
        system.mem_ctrl.dram = LPDDR3_1600_x32()
    elif dram_type == "HBM_1000_4H_1x64":
        system.mem_ctrl.dram = HBM_1000_4H_1x64()
    elif dram_type == "GDDR5_4000_x32":
        system.mem_ctrl.dram = GDDR5_4000_x32()
    else:
        print(f"Unknown DRAM type: {dram_type}, defaulting to DDR4_2400_8x8")
        system.mem_ctrl.dram = DDR4_2400_8x8()
    
    system.mem_ctrl.dram.range = system.mem_ranges[0]
    system.mem_ctrl.port = system.membus.mem_side_ports
    
    print("System configuration complete")

if __name__ == "__main__":
    main()