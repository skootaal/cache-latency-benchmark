#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* High-resolution timer using RDTSC */
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>

static inline unsigned long long rdtsc(void) {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}
#else
/* Fallback for non-x86 architectures using clock_gettime */
static inline unsigned long long rdtsc(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)(ts.tv_sec * 1e9 + ts.tv_nsec);
}
#endif

/* CPU frequency detection */
static double get_cpu_frequency_ghz(void) {
    FILE *fp;
    char line[256];
    double freq_mhz = 2400.0; /* Default fallback */
    
    /* Try to read from /proc/cpuinfo */
    fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "cpu MHz : %lf", &freq_mhz) == 1) {
                break;
            }
        }
        fclose(fp);
    }
    
    return freq_mhz / 1000.0;
}

/* Calibrate the timer by measuring RDTSC overhead */
static double calibrate_rdtsc(double cpu_freq_ghz) {
    unsigned long long start, end;
    int iterations = 1000;
    
    start = rdtsc();
    for (int i = 0; i < iterations; i++) {
        __asm__ __volatile__ ("nop");
    }
    end = rdtsc();
    
    double cycles_per_iteration = (double)(end - start) / iterations;
    double ns_per_cycle = 1000.0 / cpu_freq_ghz;
    
    return cycles_per_iteration * ns_per_cycle;
}

/* Measure L1 cache latency (small array, ~32KB) */
static double measure_l1_latency(double cpu_freq_ghz) {
    const int array_size = 8192; /* 32KB / 4 bytes per int */
    int *array = (int *)malloc(array_size * sizeof(int));
    
    /* Initialize array with pointer chasing pattern */
    int *indices = (int *)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        indices[i] = (i + 1) % array_size;
    }
    
    /* Warm up the cache */
    volatile int dummy = 0;
    for (int i = 0; i < array_size; i++) {
        dummy += array[i];
    }
    
    /* Measure latency */
    unsigned long long start = rdtsc();
    int index = 0;
    for (int i = 0; i < 100000; i++) {
        index = indices[index];
        dummy += array[index];
    }
    unsigned long long end = rdtsc();
    
    double cycles = (double)(end - start) / 100000;
    double ns_per_cycle = 1000.0 / cpu_freq_ghz;
    double latency_ns = cycles * ns_per_cycle;
    
    free(array);
    free(indices);
    
    return latency_ns;
}

/* Measure L2 cache latency (medium array, ~256KB) */
static double measure_l2_latency(double cpu_freq_ghz) {
    const int array_size = 65536; /* 256KB / 4 bytes per int */
    int *array = (int *)malloc(array_size * sizeof(int));
    
    /* Initialize array with pointer chasing pattern */
    int *indices = (int *)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        indices[i] = (i + 1) % array_size;
    }
    
    /* Warm up the cache */
    volatile int dummy = 0;
    for (int i = 0; i < array_size; i++) {
        dummy += array[i];
    }
    
    /* Measure latency */
    unsigned long long start = rdtsc();
    int index = 0;
    for (int i = 0; i < 100000; i++) {
        index = indices[index];
        dummy += array[index];
    }
    unsigned long long end = rdtsc();
    
    double cycles = (double)(end - start) / 100000;
    double ns_per_cycle = 1000.0 / cpu_freq_ghz;
    double latency_ns = cycles * ns_per_cycle;
    
    free(array);
    free(indices);
    
    return latency_ns;
}

/* Measure L3 cache latency (large array, ~8MB) */
static double measure_l3_latency(double cpu_freq_ghz) {
    const int array_size = 2097152; /* 8MB / 4 bytes per int */
    int *array = (int *)malloc(array_size * sizeof(int));
    
    /* Initialize array with pointer chasing pattern */
    int *indices = (int *)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        indices[i] = (i + 1) % array_size;
    }
    
    /* Warm up the cache */
    volatile int dummy = 0;
    for (int i = 0; i < array_size / 10; i++) {
        dummy += array[i];
    }
    
    /* Measure latency */
    unsigned long long start = rdtsc();
    int index = 0;
    for (int i = 0; i < 100000; i++) {
        index = indices[index];
        dummy += array[index];
    }
    unsigned long long end = rdtsc();
    
    double cycles = (double)(end - start) / 100000;
    double ns_per_cycle = 1000.0 / cpu_freq_ghz;
    double latency_ns = cycles * ns_per_cycle;
    
    free(array);
    free(indices);
    
    return latency_ns;
}

/* Measure main memory latency (very large array, >100MB) */
static double measure_memory_latency(double cpu_freq_ghz) {
    const int array_size = 33554432; /* 128MB / 4 bytes per int */
    int *array = (int *)malloc(array_size * sizeof(int));
    
    /* Initialize array with pointer chasing pattern */
    int *indices = (int *)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        indices[i] = (i + 1) % array_size;
    }
    
    /* Warm up (minimal) */
    volatile int dummy = 0;
    dummy += array[0];
    
    /* Measure latency */
    unsigned long long start = rdtsc();
    int index = 0;
    for (int i = 0; i < 10000; i++) {
        index = indices[index];
        dummy += array[index];
    }
    unsigned long long end = rdtsc();
    
    double cycles = (double)(end - start) / 10000;
    double ns_per_cycle = 1000.0 / cpu_freq_ghz;
    double latency_ns = cycles * ns_per_cycle;
    
    free(array);
    free(indices);
    
    return latency_ns;
}

int main(int argc, char *argv[]) {
    printf("=====================================\n");
    printf("  CPU Cache & Memory Latency Benchmark\n");
    printf("=====================================\n\n");
    
    /* Get CPU frequency */
    double cpu_freq_ghz = get_cpu_frequency_ghz();
    printf("Detected CPU Frequency: %.2f GHz\n", cpu_freq_ghz);
    printf("Nanoseconds per cycle: %.3f ns\n\n", 1000.0 / cpu_freq_ghz);
    
    /* Calibrate RDTSC */
    double rdtsc_overhead = calibrate_rdtsc(cpu_freq_ghz);
    printf("RDTSC overhead: %.2f ns\n\n", rdtsc_overhead);
    
    /* Run measurements */
    printf("Measuring cache latencies...\n");
    printf("(This may take a minute)\n\n");
    
    double l1_latency = measure_l1_latency(cpu_freq_ghz);
    printf("L1 Cache Latency:      %.2f ns\n", l1_latency);
    
    double l2_latency = measure_l2_latency(cpu_freq_ghz);
    printf("L2 Cache Latency:      %.2f ns\n", l2_latency);
    
    double l3_latency = measure_l3_latency(cpu_freq_ghz);
    printf("L3 Cache Latency:      %.2f ns\n", l3_latency);
    
    double mem_latency = measure_memory_latency(cpu_freq_ghz);
    printf("Main Memory Latency:   %.2f ns\n", mem_latency);
    
    printf("\n=====================================\n");
    printf("Relative Latencies (L1 = 1x):\n");
    printf("L1:            1.0x\n");
    printf("L2:            %.1fx\n", l2_latency / l1_latency);
    printf("L3:            %.1fx\n", l3_latency / l1_latency);
    printf("Main Memory:   %.1fx\n", mem_latency / l1_latency);
    printf("=====================================\n");
    
    return 0;
}
