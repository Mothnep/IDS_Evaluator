import m5
from m5.objects import *
import argparse
import json
import os

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

# Parse arguments - similar to arm_script.py
parser = argparse.ArgumentParser()
parser.add_argument("--config", type=str, default="simulation_config.json", 
                    help="Configuration file")
parser.add_argument("--config-name", type=str, default="default",
                    help="Name of the configuration to use")
args = parser.parse_args()

# Load the full configuration - similar to arm_script.py
config_path = os.path.join(os.path.dirname(__file__), args.config)
try:
    # Extract the full configuration
    config = load_config(config_path, args.config_name)
    print(f"Loaded configuration: {config_path}, using profile: {args.config_name}")
    
    # Extract CPU type from config
    cpu_type = config["cpu"]["type"]
    print(f"Using CPU type from config: {cpu_type}")
except Exception as e:
    print(f"Error loading configuration: {e}. Using default CPU type.")
    cpu_type = "ArmTimingSimpleCPU"
    config = {}

#This whole file provides the hardware simulation which will run our application defined elsewhere
#These processes work using SimObjects, which are the basic building blocks of gem5
system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('512MB')]

# Only use the CPU type from the loaded config
if cpu_type == "TimingSimpleCPU":
    system.cpu = ArmTimingSimpleCPU()
elif cpu_type == "AtomicSimpleCPU":
    system.cpu = ArmAtomicSimpleCPU()
elif cpu_type == "O3CPU" or cpu_type == "DerivO3CPU":
    system.cpu = ArmDerivO3CPU()
elif cpu_type == "MinorCPU":
    system.cpu = ArmMinorCPU()
else:
    # Default to ArmTimingSimpleCPU for any other value
    system.cpu = ArmTimingSimpleCPU()
    
system.membus = SystemXBar()

system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports


system.cpu.createInterruptController()
system.system_port = system.membus.cpu_side_ports # Requeston on left, response on right

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

# Create a process and assign it to the CPU
binary = '/home/mothnep/Desktop/IDS_evaluation_tool/simulators/gem5/tests/test-progs/hello/bin/arm/linux/hello'

# for gem5 V21 and beyond
system.workload = SEWorkload.init_compatible(binary)

process = Process()
process.cmd = [binary]
system.cpu.workload = process
system.cpu.createThreads()

#Instantiate system and begin execution
root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()

print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))


