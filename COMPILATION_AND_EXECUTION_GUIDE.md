# Cache and Memory Latency Benchmark - SLES15 SP7 Setup Guide

## Overview
This benchmark program measures CPU cache (L1, L2, L3) and main memory latency on x86-64 systems with nanosecond precision using RDTSC (Read Time Stamp Counter).

## Prerequisites for SLES15 SP7

### 1. Install Required Development Tools
```bash
sudo zypper refresh
sudo zypper install -y gcc make glibc-devel kernel-devel
```

### 2. Verify GCC Installation
```bash
gcc --version
```
Expected output: GCC 7.x or higher (SLES15 SP7 ships with GCC 7+)

## Compilation Instructions

### Basic Compilation (Recommended)
```bash
gcc -O2 -march=native -fno-strict-aliasing cache_latency.c -o cache_latency -lm
```

### Compilation Options Explained
| Flag | Purpose |
|------|---------|
| `-O2` | Optimization level 2 (good balance between speed and compile time) |
| `-march=native` | Optimize for the current CPU (enables RDTSC and other CPU features) |
| `-fno-strict-aliasing` | Disables strict aliasing rules (safer for pointer chasing) |
| `-lm` | Link math library (for floating-point operations) |

### Advanced Compilation (Maximum Performance)
```bash
gcc -O3 -march=native -fno-strict-aliasing -flto cache_latency.c -o cache_latency -lm
```

### Compilation for Generic x86-64 (If native fails)
```bash
gcc -O2 -march=x86-64 -fno-strict-aliasing cache_latency.c -o cache_latency -lm
```

## Execution Instructions

### Step 1: Run the Benchmark
```bash
./cache_latency
```

### Step 2: Expected Output
```
=====================================
  CPU Cache & Memory Latency Benchmark
=====================================

Detected CPU Frequency: 2.40 GHz
Nanoseconds per cycle: 0.417 ns

RDTSC overhead: 15.42 ns

Measuring cache latencies...
(This may take a minute)

L1 Cache Latency:      4.23 ns
L2 Cache Latency:      12.54 ns
L3 Cache Latency:      42.18 ns
Main Memory Latency:   120.45 ns

=====================================
Relative Latencies (L1 = 1x):
L1:            1.0x
L2:            2.96x
L3:            9.97x
Main Memory:   28.46x
=====================================
```

### Step 3: Run Multiple Times for Validation
```bash
for i in {1..5}; do
    echo "=== Run $i ==="
    ./cache_latency
    sleep 1
done
```

## Performance Tips for Accurate Measurements

### 1. Disable Dynamic Frequency Scaling (Optional but Recommended)
```bash
# Check current governor
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# Switch to performance mode (requires root)
sudo bash -c 'echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor'

# Run benchmark
./cache_latency

# Switch back to powersave
sudo bash -c 'echo powersave | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor'
```

### 2. Minimize System Load
```bash
# Close unnecessary applications
# Run benchmark when system is idle
top  # Check system load before running

./cache_latency
```

### 3. Disable CPU Turbo Boost (Optional)
```bash
# Check turbo boost status
cat /sys/devices/system/cpu/cpu0/cpufreq/boost

# Disable (if running benchmark for consistency)
sudo bash -c 'echo 0 > /sys/devices/system/cpu/cpufreq/boost'

# Run benchmark
./cache_latency

# Re-enable
sudo bash -c 'echo 1 > /sys/devices/system/cpu/cpufreq/boost'
```

## Interpreting Results

### Typical Latency Values (Intel/AMD x86-64)
| Cache Level | Typical Latency | Notes |
|-------------|-----------------|-------|
| L1 | 4-5 ns | Fastest, on-core cache |
| L2 | 10-15 ns | Per-core cache |
| L3 | 40-75 ns | Shared cache (8-20MB typical) |
| Main Memory | 60-150 ns | System RAM access |

### What Good Results Look Like
- L2 should be 2.5x - 4x L1
- L3 should be 10x - 20x L1
- Memory should be 15x - 50x L1
- Ratio consistency across multiple runs indicates accurate measurements

### Troubleshooting

**Issue: Very high or inconsistent results**
- Disable frequency scaling (see "Performance Tips" above)
- Close background applications
- Run multiple times and take average

**Issue: L1 latency > 10 ns**
- CPU frequency detection may be incorrect
- Check `/proc/cpuinfo` for actual CPU frequency
- Edit `cache_latency.c` and manually set `cpu_freq_ghz` value

**Issue: Compilation fails with "undefined reference"**
```bash
# Add explicit linking
gcc -O2 -march=native cache_latency.c -o cache_latency -lm -lpthread
```

**Issue: "Permission denied" when running**
```bash
chmod +x cache_latency
./cache_latency
```

## Advanced: Modifying the Benchmark

### Increase Iterations for More Stable Results
Edit `cache_latency.c` and increase loop iterations (around line 120):
```c
for (int i = 0; i < 500000; i++) {  // Changed from 100000
```

### Customize Array Sizes
For different cache levels, modify these values:
- L1: `const int array_size = 8192;` (32KB)
- L2: `const int array_size = 65536;` (256KB)
- L3: `const int array_size = 2097152;` (8MB)
- Memory: `const int array_size = 33554432;` (128MB)

## Running on SLES15 SP7 Systems with Multiple CPUs

### Pin to Specific CPU
```bash
taskset -c 0 ./cache_latency
```

### Run on All CPUs and Compare
```bash
for cpu in {0..3}; do
    echo "=== CPU $cpu ==="
    taskset -c $cpu ./cache_latency
done
```

## Output Explanation

- **Detected CPU Frequency**: Used to convert RDTSC cycles to nanoseconds
- **RDTSC overhead**: Time cost of reading the timestamp counter itself
- **L1 Cache Latency**: Time to access data in L1 cache (typically 4-5 ns)
- **L2 Cache Latency**: Time to access data in L2 cache (typically 10-15 ns)
- **L3 Cache Latency**: Time to access data in L3 cache (typically 40-75 ns)
- **Main Memory Latency**: Time to access data in main RAM (typically 60-150 ns)
- **Relative Latencies**: Ratio compared to L1 (shows memory hierarchy depth)

## Reference: Cache Hierarchy on Modern CPUs
```
┌─────────────────────────────────────────────┐
│  CPU Core                                   │
│  ┌─────────────────────────────────────┐    │
│  │ L1 Instruction Cache (32KB)         │    │ ~4 ns
│  │ L1 Data Cache (32KB)                │    │
│  ├─────────────────────────────────────┤    │
│  │ L2 Unified Cache (256KB per core)   │    │ ~12 ns
│  └─────────────────────────────────────┘    │
│  ┌─────────────────────────────────────┐    │
│  │ L3 Shared Cache (8-20MB)            │    │ ~40 ns
│  └─────────────────────────────────────┘    │
└─────────────────────────────────────────────┘
  │
  └──→ Main Memory (8GB+)                      ~100 ns
```

## License
This benchmark is provided as-is for educational and performance analysis purposes.
