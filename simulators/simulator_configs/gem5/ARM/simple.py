import m5
from m5.objects import *
from m5.util import addToPath
import argparse
import json
import os
import sys

# Add gem5 configs to path
addToPath('/home/mothnep/Desktop/IDS_evaluation_tool/simulators/gem5/configs')

# Import required gem5 scripts
from common import ObjectList
from common import MemConfig
from common.cores.arm import HPI

def loadConfig(config_file):
    """Load configuration from JSON file"""
    with open(config_file, 'r') as f:
        config = json.load(f)
    return config["configurations"]["default"]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", type=str, default="simulation_config.json",
                        help="Configuration file")
    args = parser.parse_args()
    
    # Load configuration
    config_path = os.path.join(os.path.dirname(__file__), args.config)
    config = loadConfig(config_path)
    print(f"Loaded configuration: {config_path}")
    
    # Create the system
    system = System()
    
    # Clock domain settings
    system.clk_domain = SrcClockDomain()
    system.clk_domain.clock = config["system"]["clock_domain"]["clock"]
    system.clk_domain.voltage_domain = VoltageDomain()
    system.clk_domain.voltage_domain.voltage = config["system"]["clock_domain"]["voltage_domain"]["voltage"]
    
    # Memory mode
    system.mem_mode = config["system"]["mem_mode"]
    
    # Memory configuration
    system.mem_ranges = [AddrRange(config["memory"]["size"])]
    
    # Get CPU configuration
    cpu_config = config["cpu"]
    cpu_type = cpu_config["type"]
    num_cores = cpu_config["num_cores"]
    
    # Create CPU(s) based on num_cores
    system.cpu = [None] * num_cores
    
    # Check if branch predictor configuration exists
    has_bp_config = "branch_predictor" in cpu_config
    
    # Create and connect CPUs
    for i in range(num_cores):
        # Select CPU class based on type (remove "Arm" prefix from JSON config names)
        if cpu_type == "ArmTimingSimpleCPU":
            system.cpu[i] = TimingSimpleCPU()
        elif cpu_type == "ArmAtomicSimpleCPU":
            system.cpu[i] = AtomicSimpleCPU()
        elif cpu_type == "ArmO3CPU":
            system.cpu[i] = DerivO3CPU()
            
            # Configure branch predictor for O3CPU if specified
            if has_bp_config:
                bp_config = cpu_config["branch_predictor"]
                bp_type = bp_config.get("type", "TournamentBP")
                
                if bp_type == "LocalBP":
                    system.cpu[i].branchPred = LocalBP()
                elif bp_type == "TournamentBP":
                    system.cpu[i].branchPred = TournamentBP()
                elif bp_type == "BiModeBP":
                    system.cpu[i].branchPred = BiModeBP()
                elif bp_type == "LTAGE":
                    system.cpu[i].branchPred = LTAGE()
                else:
                    print(f"Warning: Unknown branch predictor type: {bp_type}, using TournamentBP")
                    system.cpu[i].branchPred = TournamentBP()
                
                # Apply any BP-specific parameters
                for param, value in bp_config.items():
                    if param != "type" and hasattr(system.cpu[i].branchPred, param):
                        setattr(system.cpu[i].branchPred, param, value)
                        
        elif cpu_type == "ArmMinorCPU":
            system.cpu[i] = MinorCPU()
            
            # Configure branch predictor for MinorCPU if specified
            if has_bp_config:
                bp_config = cpu_config["branch_predictor"]
                bp_type = bp_config.get("type", "TournamentBP")
                
                if bp_type == "LocalBP":
                    system.cpu[i].branchPred = LocalBP()
                elif bp_type == "TournamentBP":
                    system.cpu[i].branchPred = TournamentBP()
                elif bp_type == "BiModeBP":
                    system.cpu[i].branchPred = BiModeBP()
                elif bp_type == "LTAGE":
                    system.cpu[i].branchPred = LTAGE()
                else:
                    print(f"Warning: Unknown branch predictor type: {bp_type}, using TournamentBP")
                    system.cpu[i].branchPred = TournamentBP()
                
                # Apply any BP-specific parameters
                for param, value in bp_config.items():
                    if param != "type" and hasattr(system.cpu[i].branchPred, param):
                        setattr(system.cpu[i].branchPred, param, value)
                        
        elif cpu_type == "ArmHPI":
            system.cpu[i] = HPI.HPI()
    
            # HPI is based on MinorCPU, so it supports branch prediction
            if has_bp_config:
                bp_config = cpu_config["branch_predictor"]
                bp_type = bp_config.get("type", "TournamentBP")
                
                if bp_type == "LocalBP":
                    system.cpu[i].branchPred = LocalBP()
                elif bp_type == "TournamentBP":
                    system.cpu[i].branchPred = TournamentBP()
                elif bp_type == "BiModeBP":
                    system.cpu[i].branchPred = BiModeBP()
                elif bp_type == "LTAGE":
                    system.cpu[i].branchPred = LTAGE()
                else:
                    print(f"Warning: Unknown branch predictor type: {bp_type}, using TournamentBP")
                    system.cpu[i].branchPred = TournamentBP()
                
                # Apply BP-specific parameters
                for param, value in bp_config.items():
                    if param != "type" and hasattr(system.cpu[i].branchPred, param):
                        setattr(system.cpu[i].branchPred, param, value)
        else:
            print(f"Unknown CPU type: {cpu_type}")
            sys.exit(1)
    
    # Create caches
    cache_config = config["cache_hierarchy"]
    
    # L1 caches for each CPU
    for i in range(num_cores):
        system.cpu[i].icache = L1_ICache(size=cache_config["l1i_size"], 
                                       assoc=cache_config["l1i_assoc"])
        system.cpu[i].dcache = L1_DCache(size=cache_config["l1d_size"],
                                       assoc=cache_config["l1d_assoc"])
        
        # Connect caches to CPU
        system.cpu[i].icache.connectCPU(system.cpu[i])
        system.cpu[i].dcache.connectCPU(system.cpu[i])
    
    # Shared L2 cache and crossbar
    system.l2bus = L2XBar()
    system.l2cache = L2Cache(size=cache_config["l2_size"],
                           assoc=cache_config["l2_assoc"])
    
    # Connect L1 caches to L2 bus
    for i in range(num_cores):
        system.cpu[i].icache.connectBus(system.l2bus)
        system.cpu[i].dcache.connectBus(system.l2bus)
    
    # Connect L2 cache to the L2 bus
    system.l2cache.connectCPUSideBus(system.l2bus)
    
    # System interconnect
    system.membus = SystemXBar()
    system.l2cache.connectMemSideBus(system.membus)
    
    # Create memory controller
    system.mem_ctrl = MemCtrl()  # Simplify to always use MemCtrl for SE mode
    
    # Configure DRAM using specified type
    dram_type = config["memory"]["dram"]["type"]
    
    # Use ObjectList to find the DRAM class
    try:
        dram_class = ObjectList.mem_list.get(dram_type)
        system.mem_ctrl.dram = dram_class()
    except:
        print(f"Unknown DRAM type: {dram_type}")
        print(f"Available types: {ObjectList.mem_list.get_names()}")
        sys.exit(1)
    
    # Only set DRAM parameters that are universally supported
    system.mem_ctrl.dram.range = system.mem_ranges[0]
    system.mem_ctrl.port = system.membus.mem_side_ports
    
    # Create the process
    process_config = config["process"]
    binary_path = process_config["binary"]
    
    system.workload = SEWorkload.init_compatible(binary_path)
    
    # Create and set the process
    process = Process()
    process.cmd = [binary_path] + process_config["args"]
    process.output = process_config["output"]
    
    # Configure environment variables if specified
    if process_config["env"] and len(process_config["env"]) > 0:
        process.env = process_config["env"]
    
    # Configure input redirection if specified
    if process_config["input"] and process_config["input"] != "":
        process.input = process_config["input"]
    
    # Assign workload to all CPUs
    for i in range(num_cores):
        system.cpu[i].workload = process
        system.cpu[i].createThreads()
    
    # Connect the system ports
    system.system_port = system.membus.cpu_side_ports
    
    # Create the root and instantiate the system
    root = Root(full_system=False, system=system)  # SE mode
    
    # Simulate
    print(f"Beginning simulation with binary: {binary_path}")
    m5.instantiate()
    print("Simulation instantiated")
    
    exit_event = m5.simulate()
    print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")

if __name__ == "__main__":
    main()