# ARM Configuration Documentation for gem5

This document describes the configuration options available in the `simulation_config.json` file for ARM processor simulations in gem5.

## Configuration Structure

The configuration file defines parameters for ARM processor simulations organized in a hierarchical structure.

## System Parameters

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `mem_mode` | Memory access simulation mode | `"atomic"` (fastest, least accurate)<br>`"timing"` (balance of speed/accuracy)<br>`"detailed"` (most accurate) |
| `clock_domain.clock` | CPU clock frequency | `"1GHz"`, `"2GHz"`, `"3GHz"`, `"4GHz"` |
| `clock_domain.voltage_domain.voltage` | CPU voltage | `"0.8V"` to `"1.2V"` |
| `isa_checker` | Verify instruction execution | `true` (slower), `false` (faster) |

## CPU Parameters

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `type` | CPU model | `"ArmAtomicSimpleCPU"` (fast functional)<br>`"ArmTimingSimpleCPU"` (simple timing model)<br>`"ArmO3CPU"` (out-of-order)<br>`"ArmMinorCPU"` (in-order pipeline) |
| `num_cores` | Number of CPU cores | `1`, `2`, `4`, `8` |
| `icache_size` | Instruction cache size | `"16kB"`, `"32kB"`, `"64kB"` |
| `dcache_size` | Data cache size | `"16kB"`, `"32kB"`, `"64kB"` |

## Memory Parameters

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `controller` | Memory controller type | `"MemCtrl"`, `"DRAMCtrl"`, `"NVMCtrl"` |
| `dram.type` | DRAM technology | `"DDR3_1600_8x8"`, `"DDR3_2133_8x8"`, `"DDR4_2400_8x8"`, `"LPDDR3_1600_1x32"`, `"HBM_1000_4H_1x64"` |
| `dram.ranks_per_channel` | DRAM ranks per channel | `1`, `2`, `4` |
| `dram.channels` | Number of DRAM channels | `1`, `2`, `4`, `8` |
| `size` | Total system memory | `"512MB"`, `"1GB"`, `"2GB"`, `"4GB"`, `"8GB"` |
| `mem_channels` | Memory channels | `1`, `2`, `4`, `8` |

## Cache Hierarchy

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `l1i_size` | L1 instruction cache size | `"16kB"`, `"32kB"`, `"64kB"` |
| `l1i_assoc` | L1 instruction cache associativity | `2`, `4`, `8` |
| `l1d_size` | L1 data cache size | `"16kB"`, `"32kB"`, `"64kB"` |
| `l1d_assoc` | L1 data cache associativity | `2`, `4`, `8` |
| `l2_size` | L2 cache size | `"256kB"`, `"512kB"`, `"1MB"`, `"2MB"` |
| `l2_assoc` | L2 cache associativity | `4`, `8`, `16` |

## Process Parameters

| Parameter | Description | Purpose |
|-----------|-------------|---------|
| `binary` | Path to ARM executable | Cross-compiled binary to execute |
| `args` | Command line arguments | Arguments passed to the binary |
| `env` | Environment variables | Key-value pairs for environment settings |
| `input` | Input redirection file | File to use as standard input |
| `output` | Output redirection file | File to capture standard output |

## Default Configuration

The default configuration provides a balanced setup for ARM simulation with:
- ARMv8 architecture with 2GHz clock
- TimingSimpleCPU model with 1 core
- 32kB L1 caches and 1MB L2 cache
- 2GB system memory with DDR4 controller

To create additional configurations, add new entries to the `configurations` object.
