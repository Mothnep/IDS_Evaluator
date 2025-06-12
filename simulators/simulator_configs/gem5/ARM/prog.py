import m5
from m5.objects import *
import argparse
import json
import os
from m5.util import convert

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

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("--config", type=str, default="simulation_config.json", 
                    help="Configuration file")
parser.add_argument("--config-name", type=str, default="default",
                    help="Name of the configuration to use")
args = parser.parse_args()

# Load configuration 
config_path = os.path.join(os.path.dirname(__file__), args.config)

# Extract full configuration
config = load_config(config_path, args.config_name)

#Extract memory mode from config
mem_mode = config["system"]["mem_mode"]

# Extract clock frequency from config
clock_freq = config["system"]["clock_domain"]["clock"]

#Extract voltage from config
voltage = config["system"]["clock_domain"]["voltage_domain"]["voltage"]

# Extract memory size from config
mem_size = config["memory"]["dram"]["size"]

# Extract CPU type from config
cpu_type = config["cpu"]["type"]

# Extract number of cores from config
num_cores = config["cpu"]["num_cores"]

# Extract DRAM type
dram_type = config["memory"]["dram"]["type"]
    
# Extract binary path from config
binary = config["process"]["binary"]

def print_config(config_path, mem_mode, clock_freq, voltage, mem_size, cpu_type, num_cores, dram_type, binary):
    print(f"Configuration file: {config_path}")
    print(f"Memory mode: {mem_mode}")
    print(f"Clock frequency: {clock_freq}")
    print(f"Voltage: {voltage}")
    print(f"Memory size: {mem_size}")
    print(f"CPU type: {cpu_type}")
    print(f"Number of cores: {num_cores}")
    print(f"DRAM type: {dram_type}")
    print(f"Binary: {binary}")
    print(f"Actual CPU count: {len(system.cpu)}")
    print(f"CPU IDs: {[cpu.cpu_id for cpu in system.cpu]}")

def set_dram_size(dram_obj):
    # For MemorySize objects
    if hasattr(dram_obj.device_size, "value"):
        # Value given in Bytes,convert to MiB 
        size_dram_MiB = dram_obj.device_size.value / 1024 / 1024
        nb_devices_rank = dram_obj.devices_per_rank
        nb_ranks_channel = dram_obj.ranks_per_channel
        total_size = size_dram_MiB * nb_devices_rank * nb_ranks_channel
        system.mem_ranges = [AddrRange(str(int(total_size)) + "MiB")]
        #print(f"Total DRAM size: {total_size} MiB")


# Initialize system and basic parameters
system = System()

# Set clock domain and voltage domain
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = clock_freq
system.clk_domain.voltage_domain = VoltageDomain()
system.clk_domain.voltage_domain.voltage = voltage

# Set memory mode 
if mem_mode == 'timing':
    system.mem_mode = 'timing'
elif mem_mode == 'atomic':
    system.mem_mode = 'atomic'
elif mem_mode == 'functional':
    system.mem_mode = 'functional'
else:
    raise ValueError(f"Unsupported memory mode: {mem_mode}")


# Create the CPU vector properly (can't be empty)
system.cpu = [ArmTimingSimpleCPU(cpu_id=i) for i in range(num_cores)] 
# Then configure each CPU based on type if needed
for i in range(num_cores):
    if cpu_type != "TimingSimpleCPU":
        if cpu_type == "AtomicSimpleCPU":
            system.cpu[i] = ArmAtomicSimpleCPU(cpu_id=i)
        elif cpu_type == "O3CPU":
            system.cpu[i] = ArmO3CPU(cpu_id=i)
        elif cpu_type == "MinorCPU":
            system.cpu[i] = ArmMinorCPU(cpu_id=i)


system.membus = SystemXBar()
# Connect each CPU's cache ports to the membus
for cpu in system.cpu:
    cpu.icache_port = system.membus.cpu_side_ports
    cpu.dcache_port = system.membus.cpu_side_ports
    cpu.createInterruptController()

system.system_port = system.membus.cpu_side_ports # Request on on left, response on right

# Memory controller setup (needs to be in a loop if wanting to increase number of dram channels)
system.mem_ctrl = MemCtrl()
# Create the DRAM object dynamically
system.mem_ctrl.dram = eval(dram_type)()

# Set the DRAM complete size and range
set_dram_size(system.mem_ctrl.dram)
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

# Create a process and assign it to the CPU
system.workload = SEWorkload.init_compatible(binary)

process = Process()
process.cmd = [binary]

# Assign the process to each CPU and create threads
for cpu in system.cpu:
    cpu.workload = process
    cpu.createThreads()

#Instantiate system and begin execution
root = Root(full_system = False, system = system)
m5.instantiate()

print_config(config_path, mem_mode, clock_freq, voltage, mem_size, cpu_type, num_cores, dram_type, binary)

print("Beginning simulation!")
exit_event = m5.simulate()

print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))





