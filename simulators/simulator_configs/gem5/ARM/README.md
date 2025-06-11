# ARM Configuration Documentation for gem5

This document describes the configuration options available in the `simulation_config.json` file for ARM processor simulations in gem5.

## Configuration Structure

The configuration file defines parameters for ARM processor simulations organized in a hierarchical structure.

## System Parameters

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `mem_mode` | Memory access simulation mode<br>(Needs to fit CPU type) | `"atomic"` (warming up)<br>`"timing"` (only suitable for simulations)<br>`"functional"` (debugging mode) |
| `clock_domain.clock` | CPU clock frequency | Correct Syntax: `"1GHz"`, `"2GHz"`, `"3GHz"`, `"4GHz"`, etc... |
| `clock_domain.voltage_domain.voltage` | CPU voltage | Correct Sytax: `"0.8V"`, `"1.2V"`, etc... |

## CPU Parameters

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `type` | CPU model | `"AtomicSimpleCPU"` (Requests instantly finished)<br>`"TimingSimpleCPU"` (Memory access but no pipelining)<br>`"O3CPU"` (out-of-order)<br>`"MinorCPU"` (in-order) |
| `num_cores` | Number of CPU cores | Correct Syntax: `1`, `2`, `4`, `8`, etc... |
| `icache_size` | Instruction cache size | Correct Syntax: `"16kB"`, `"32kB"`, `"64kB"`, etc... |
| `dcache_size` | Data cache size | Correct Syntax:  `"16kB"`, `"32kB"`, `"64kB"`, etc... |

## Memory Parameters

| Parameter | Description | Available Options |
|-----------|-------------|-------------------|
| `dram.type` | DRAM technology | `"DDR3_1600_8x8"`, `"DDR3_2133_8x8"`, `"DDR4_2400_16x4"`, `"DDR4_2400_8x8"`, `"DDR4_2400_4x16"`, `"DDR5_4400_4x8"`, `"DDR5_6400_4x8"`, `"DDR5_4800_4x8"`, `"HBM_1000_4H_1x128"`, `"HBM_1000_4H_1x64"`, `"HBM_2000_4H_1x64"`, `"HMC_2500_1x32"`, `"GDDR5_4000_2x32"`, `"LPDDR2_S4_1066_1x32"`, `"LPDDR3_1600_1x32"`, `"LPDDR5_5500_1x16_BG_BL32"`, `"LPDDR5_5500_1x16_BG_BL16"`, `"LPDDR5_5500_1x16_8B_BL32"`, `"LPDDR5_6400_1x16_BG_BL32"`, `"LPDDR5_6400_1x16_BG_BL16"`, `"LPDDR5_6400_1x16_8B_BL32"`, |
| `dram.ranks_per_channel` | DRAM ranks per channel | `1`, `2`, `4` |
| `dram.channels` | Number of DRAM channels (not operational for now) | `1`, `2`, `4`, `8` |
| `size` | Single Dram size | `"512MB"`, `"1GB"`, `"2GB"`, `"4GB"`, `"8GB"` |

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


## Default Configuration

The default configuration provides a balanced setup for ARM simulation with:
- ARMv8 architecture with 2GHz clock
- TimingSimpleCPU model with 1 core
- 32kB L1 caches and 1MB L2 cache
- 2GB system memory with DDR4 controller

To create additional configurations, add new entries to the `configurations` object.
