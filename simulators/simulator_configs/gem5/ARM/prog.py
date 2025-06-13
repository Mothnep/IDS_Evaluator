import m5
from m5.objects import *
import argparse
import json
import os
from m5.util import convert
from caches import *

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
system.membus = SystemXBar()

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


# Create an options object to pass to the cache constructors
class Options(object):
    pass
    
options = Options()

# Extract cache configuration from JSON and populate options object
if "cache_hierarchy" in config:
    cache_config = config["cache_hierarchy"]
    
    # General L1 cache parameters (apply to both I and D caches)
    if "l1" in cache_config:
        if "tag_latency" in cache_config["l1"]:
            options.l1_tag_latency = cache_config["l1"]["tag_latency"]
        if "data_latency" in cache_config["l1"]:
            options.l1_data_latency = cache_config["l1"]["data_latency"]
        if "response_latency" in cache_config["l1"]:
            options.l1_response_latency = cache_config["l1"]["response_latency"]
        if "mshrs" in cache_config["l1"]:
            options.l1_mshrs = cache_config["l1"]["mshrs"]
        if "tgts_per_mshr" in cache_config["l1"]:
            options.l1_tgts_per_mshr = cache_config["l1"]["tgts_per_mshr"]
    
    # L1I cache parameters - keep only size and associativity
    if "l1i" in cache_config:
        options.l1i_size = cache_config["l1i"]["size"]
        options.l1i_assoc = cache_config["l1i"]["assoc"]
        #Overarching parameters can be made specific to L1I cache here if needed
    
    # L1D cache parameters - keep only size and associativity
    if "l1d" in cache_config:
        options.l1d_size = cache_config["l1d"]["size"]
        options.l1d_assoc = cache_config["l1d"]["assoc"]
        #Overarching parameters can be made specific to L1D cache here if needed
    
    # L2 cache parameters
    if "l2" in cache_config:
        options.l2_size = cache_config["l2"]["size"]
        options.l2_assoc = cache_config["l2"]["assoc"]  
        options.l2_tag_latency = cache_config["l2"]["tag_latency"]
        options.l2_data_latency = cache_config["l2"]["data_latency"]
        options.l2_response_latency = cache_config["l2"]["response_latency"]
        options.l2_mshrs = cache_config["l2"]["mshrs"]
        options.l2_tgts_per_mshr = cache_config["l2"]["tgts_per_mshr"]

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

# Create L2 bus first (needed for connections because L2 expects a single connection)
system.l2bus = L2XBar()

# Create caches for each CPU first
for i in range(num_cores):
    # Create caches for this CPU
    system.cpu[i].icache = L1ICache(options=options)
    system.cpu[i].dcache = L1DCache(options=options)
    
    # Connect the caches to the CPU ports
    system.cpu[i].icache.connectCPU(system.cpu[i])
    system.cpu[i].dcache.connectCPU(system.cpu[i])
    
    # Connect the caches to the L2 bus
    system.cpu[i].icache.connectBus(system.l2bus)
    system.cpu[i].dcache.connectBus(system.l2bus)
    
    # Create interrupt controller for this CPU
    system.cpu[i].createInterruptController()

# Now create and connect the L2 cache
system.l2cache = L2Cache(options=options)
system.l2cache.connectCPUSideBus(system.l2bus)
system.l2cache.connectMemSideBus(system.membus)

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





