# TurboKit Performance Guide

Comprehensive guide to maximizing performance with TurboKit components.

## Table of Contents

- [Performance Philosophy](#performance-philosophy)
- [Measurement and Profiling](#measurement-and-profiling)
- [Component-Specific Optimizations](#component-specific-optimizations)
- [Memory Management](#memory-management)
- [Cache Optimization](#cache-optimization)
- [Compiler Optimizations](#compiler-optimizations)
- [Platform-Specific Tuning](#platform-specific-tuning)
- [Common Performance Pitfalls](#common-performance-pitfalls)
- [Benchmarking Best Practices](#benchmarking-best-practices)

---

## Performance Philosophy

TurboKit is built on three core performance principles:

### 1. **Predictable Performance**
Every operation has deterministic time complexity with minimal variance.

```cpp
// ✅ Predictable: Pre-allocated containers
turbokit::Vector<int> data;
data.reserve(1000000);  // Single allocation
for (int i = 0; i < 1000000; ++i) {
    data.append(i);     // Guaranteed O(1)
}

// ❌ Unpredictable: Growth-based allocation
std::vector<int> data;
for (int i = 0; i < 1000000; ++i) {
    data.push_back(i);  // Occasional O(n) reallocations
}
```

### 2. **Cache Efficiency**
Data structures are designed for optimal memory access patterns.

```cpp
// ✅ Cache-friendly: Sequential access
for (size_t i = 0; i < vec.size(); ++i) {
    process(vec[i]);
}

// ❌ Cache-unfriendly: Random access
for (auto& key : random_keys) {
    process(map[key]);
}
```

### 3. **Zero-Cost Abstractions**
High-level interfaces compile to optimal machine code.

```cpp
// Both compile to identical assembly with -O3
turbokit::Vector<int> vec;
vec.append(42);

int* ptr = (int*)malloc(sizeof(int));
*ptr = 42;
```

---

## Measurement and Profiling

### High-Precision Timing

TurboKit's Clock provides sub-microsecond precision for accurate performance measurement:

```cpp
#include <turbokit/clock.h>

class PerformanceBenchmark {
private:
    int64_t start_time_;
    const char* operation_name_;

public:
    PerformanceBenchmark(const char* name)
        : operation_name_(name) {
        // Warm up the clock to avoid calibration overhead
        for (int i = 0; i < 10; ++i) {
            turbokit::clock.get_current_time();
        }
        start_time_ = turbokit::clock.get_current_time();
    }

    ~PerformanceBenchmark() {
        auto end_time = turbokit::clock.get_current_time();
        auto duration_ns = end_time - start_time_;

        turbokit::log.info("%s: %lld ns (%.2f μs)",
                          operation_name_, duration_ns, duration_ns / 1000.0);
    }
};

#define BENCHMARK(name) PerformanceBenchmark _bench(name)

void example_benchmarking() {
    {
        BENCHMARK("Vector append");
        turbokit::Vector<int> vec;
        vec.reserve(100000);
        for (int i = 0; i < 100000; ++i) {
            vec.append(i);
        }
    }

    {
        BENCHMARK("HashMap lookup");
        turbokit::HashMap<int, int> map;
        map.reserve(100000);
        for (int i = 0; i < 100000; ++i) {
            map[i] = i * 2;
        }

        int sum = 0;
        for (int i = 0; i < 100000; ++i) {
            sum += map[i];
        }
    }
}
```

### Statistical Analysis

For accurate performance measurement, collect multiple samples:

```cpp
#include <turbokit/vector.h>
#include <algorithm>

class StatisticalBenchmark {
private:
    turbokit::Vector<int64_t> samples_;
    const char* operation_name_;

public:
    StatisticalBenchmark(const char* name, size_t sample_count = 1000)
        : operation_name_(name) {
        samples_.reserve(sample_count);
    }

    void add_sample(int64_t duration_ns) {
        samples_.append(duration_ns);
    }

    void analyze() {
        if (samples_.empty()) return;

        std::sort(samples_.begin(), samples_.end());

        size_t n = samples_.size();
        int64_t min_val = samples_[0];
        int64_t max_val = samples_[n - 1];
        int64_t median = samples_[n / 2];
        int64_t p95 = samples_[n * 95 / 100];
        int64_t p99 = samples_[n * 99 / 100];

        double sum = 0;
        for (auto sample : samples_) {
            sum += sample;
        }
        double mean = sum / n;

        turbokit::log.info("%s statistics (ns):", operation_name_);
        turbokit::log.info("  Samples: %zu", n);
        turbokit::log.info("  Min: %lld", min_val);
        turbokit::log.info("  Mean: %.2f", mean);
        turbokit::log.info("  Median: %lld", median);
        turbokit::log.info("  P95: %lld", p95);
        turbokit::log.info("  P99: %lld", p99);
        turbokit::log.info("  Max: %lld", max_val);
    }
};

void statistical_benchmark_example() {
    StatisticalBenchmark bench("HashMap insert", 10000);

    turbokit::HashMap<int, int> map;
    map.reserve(100000);

    for (int i = 0; i < 10000; ++i) {
        auto start = turbokit::clock.get_current_time();
        map[i] = i * 2;
        auto end = turbokit::clock.get_current_time();
        bench.add_sample(end - start);
    }

    bench.analyze();
}
```

---

## Component-Specific Optimizations

### Vector Performance

#### Pre-allocation Strategy

```cpp
// ❌ Bad: Multiple reallocations
turbokit::Vector<Item> items;
for (const auto& data : input_data) {
    items.append(process(data));  // May trigger reallocations
}

// ✅ Good: Single allocation
turbokit::Vector<Item> items;
items.reserve(input_data.size());  // Pre-allocate exact size
for (const auto& data : input_data) {
    items.append(process(data));   // Guaranteed no reallocations
}

// ✅ Better: Batch processing
turbokit::Vector<Item> items;
items.resize(input_data.size());   // Allocate and default-construct
for (size_t i = 0; i < input_data.size(); ++i) {
    items[i] = process(input_data[i]);  // Direct assignment
}
```

#### Memory Layout Optimization

```cpp
// ❌ Bad: Poor cache locality
struct BadLayout {
    std::string name;        // Variable size, heap allocation
    double value;
    int id;
    std::vector<double> data; // Another heap allocation
};

// ✅ Good: Cache-friendly layout
struct GoodLayout {
    int id;                  // 4 bytes
    float value;             // 4 bytes (total: 8 bytes, fits in cache line)
    // Store strings separately or use fixed-size arrays
};

turbokit::Vector<GoodLayout> good_data;  // Contiguous, cache-friendly
```

### HashMap Performance

#### Hash Function Quality

```cpp
// ❌ Bad: Poor hash distribution
struct BadHash {
    size_t operator()(const std::string& s) const {
        return s.length();  // Many collisions for same-length strings
    }
};

// ✅ Good: High-quality hash
struct GoodHash {
    size_t operator()(const std::string& s) const {
        // Use standard library hash or a proven algorithm
        return std::hash<std::string>{}(s);
    }
};

// ✅ Better: Custom optimized hash for specific data
struct OptimizedHash {
    size_t operator()(uint64_t key) const {
        // Fast hash for integer keys
        key ^= key >> 33;
        key *= 0xff51afd7ed558ccd;
        key ^= key >> 33;
        key *= 0xc4ceb9fe1a85ec53;
        key ^= key >> 33;
        return key;
    }
};
```

#### Load Factor Management

```cpp
// ✅ Optimal: Pre-allocate with known size
turbokit::HashMap<KeyType, ValueType> map;
map.reserve(expected_size * 1.25);  // Account for load factor

// Monitor load factor in production
void monitor_hash_performance(const auto& map) {
    double load_factor = map.load_factor();
    if (load_factor > 0.8) {
        turbokit::log.warning("High load factor: %.2f", load_factor);
    }
}
```

### Clock Optimization

#### Minimize Clock Calls

```cpp
// ❌ Bad: Clock call in tight loop
for (int i = 0; i < 1000000; ++i) {
    auto start = turbokit::clock.get_current_time();
    process_item(i);
    auto end = turbokit::clock.get_current_time();
    latencies[i] = end - start;
}

// ✅ Good: Batch timing
auto start = turbokit::clock.get_current_time();
for (int i = 0; i < 1000000; ++i) {
    process_item(i);
}
auto end = turbokit::clock.get_current_time();
auto avg_latency = (end - start) / 1000000;

// ✅ Better: Sampled timing
for (int i = 0; i < 1000000; ++i) {
    if (i % 1000 == 0) {  // Sample every 1000th iteration
        auto start = turbokit::clock.get_current_time();
        process_item(i);
        auto end = turbokit::clock.get_current_time();
        latencies[i / 1000] = end - start;
    } else {
        process_item(i);
    }
}
```

---

## Memory Management

### Object Pooling with FreeList

```cpp
#include <turbokit/freelist.h>

struct Message {
    uint64_t timestamp;
    uint32_t type;
    uint32_t size;
    char data[256];
    Message* next;  // Required for FreeList
};

class HighPerformanceMessageHandler {
private:
    using MessagePool = turbokit::FreeList<Message>;
    static constexpr size_t MAX_POOL_SIZE = 10000;

public:
    Message* allocate_message() {
        // Very fast allocation from pool
        Message* msg = MessagePool::remove_element();
        if (!msg) {
            // Fallback to heap allocation
            msg = new Message;
            turbokit::log.debug("Pool empty, allocated from heap");
        }
        return msg;
    }

    void deallocate_message(Message* msg) {
        // Very fast return to pool
        MessagePool::add_element(msg, MAX_POOL_SIZE);
    }

    void process_messages() {
        for (int i = 0; i < 1000000; ++i) {
            Message* msg = allocate_message();

            // Initialize message
            msg->timestamp = turbokit::clock.get_current_time();
            msg->type = i % 10;
            msg->size = sizeof(Message);

            // Process message
            handle_message(msg);

            // Return to pool
            deallocate_message(msg);
        }
    }
};
```

### Buffer Management

```cpp
#include <turbokit/buffer.h>

class HighPerformanceBufferManager {
private:
    turbokit::Vector<turbokit::BufferHandle> buffer_pool_;
    static constexpr size_t BUFFER_SIZE = 65536;  // 64KB
    static constexpr size_t POOL_SIZE = 100;

public:
    HighPerformanceBufferManager() {
        // Pre-allocate buffer pool
        buffer_pool_.reserve(POOL_SIZE);
        for (size_t i = 0; i < POOL_SIZE; ++i) {
            buffer_pool_.append(turbokit::Buffer::create(BUFFER_SIZE));
        }
    }

    turbokit::BufferHandle get_buffer() {
        if (!buffer_pool_.empty()) {
            auto buffer = std::move(buffer_pool_.back());
            buffer_pool_.pop_back();
            return buffer;
        }

        // Pool exhausted, allocate new buffer
        turbokit::log.debug("Buffer pool exhausted");
        return turbokit::Buffer::create(BUFFER_SIZE);
    }

    void return_buffer(turbokit::BufferHandle buffer) {
        if (buffer_pool_.size() < POOL_SIZE) {
            buffer_pool_.append(std::move(buffer));
        }
        // Otherwise, let buffer be destroyed (automatic cleanup)
    }
};
```

---

## Cache Optimization

### Data Layout Optimization

```cpp
// ❌ Bad: Poor cache utilization
struct BadDataLayout {
    std::string name;        // Heap allocation, cache miss
    double value1;
    bool flag;              // Padding issues
    double value2;
    std::vector<int> data;  // Another heap allocation
};

// ✅ Good: Cache-optimized layout
struct GoodDataLayout {
    double value1;          // 8 bytes
    double value2;          // 8 bytes
    uint32_t name_id;       // 4 bytes (use string interning)
    uint32_t flags;         // 4 bytes (pack multiple bools)
    // Total: 24 bytes, fits nicely in cache lines
};

class CacheOptimizedProcessor {
private:
    turbokit::Vector<GoodDataLayout> hot_data_;     // Frequently accessed
    turbokit::HashMap<uint32_t, std::string> names_; // Rarely accessed

public:
    void process_data() {
        // Hot loop only touches cache-friendly data
        for (const auto& item : hot_data_) {
            double result = item.value1 * item.value2;
            if (item.flags & 0x1) {
                process_result(result);
            }
        }
    }
};
```

### Memory Access Patterns

```cpp
// ✅ Sequential access (cache-friendly)
void sequential_processing(turbokit::Vector<double>& data) {
    double sum = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];  // Sequential memory access
    }
}

// ❌ Random access (cache-unfriendly)
void random_processing(turbokit::Vector<double>& data,
                      const turbokit::Vector<size_t>& indices) {
    double sum = 0.0;
    for (size_t idx : indices) {
        sum += data[idx];  // Random memory access
    }
}

// ✅ Blocked processing (better cache utilization)
void blocked_processing(turbokit::Vector<double>& data) {
    constexpr size_t BLOCK_SIZE = 1024;  // Fit in L1 cache

    for (size_t block = 0; block < data.size(); block += BLOCK_SIZE) {
        size_t end = std::min(block + BLOCK_SIZE, data.size());

        // Process block (stays in cache)
        for (size_t i = block; i < end; ++i) {
            data[i] = process_element(data[i]);
        }
    }
}
```

### Cache Line Alignment

```cpp
#include <cstddef>

// Align frequently accessed data to cache line boundaries
struct alignas(std::hardware_destructive_interference_size) CacheLineAligned {
    std::atomic<uint64_t> counter;
    // Padding to prevent false sharing
    char padding[std::hardware_destructive_interference_size - sizeof(uint64_t)];
};

class HighPerformanceCounter {
private:
    CacheLineAligned counters_[8];  // One per typical core count

public:
    void increment(size_t thread_id) {
        counters_[thread_id % 8].counter.fetch_add(1, std::memory_order_relaxed);
    }

    uint64_t total() const {
        uint64_t sum = 0;
        for (const auto& counter : counters_) {
            sum += counter.counter.load(std::memory_order_relaxed);
        }
        return sum;
    }
};
```

---

## Compiler Optimizations

### Build Configuration

```cmake
# CMakeLists.txt for optimal performance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release build optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")

# Additional optimizations for GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer")
endif()

# Link-time optimization
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
```

### Profile-Guided Optimization

```cmake
# Step 1: Build with instrumentation
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-generate")

# Step 2: Run typical workload to generate profile data
# Step 3: Rebuild with profile-guided optimization
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-use")
```

### Function Attributes

```cpp
// Hot path optimization
[[gnu::hot]] [[gnu::always_inline]]
inline double fast_computation(double x) {
    return x * x + 2.0 * x + 1.0;
}

// Cold path optimization
[[gnu::cold]] [[gnu::noinline]]
void error_handler(const char* message) {
    turbokit::log.error("Error: %s", message);
    // Error handling code
}

// Branch prediction hints
bool process_item(const Item& item) {
    if ([[likely]] item.is_valid()) {
        return fast_computation(item.value) > 0.0;
    } else {
        [[unlikely]];
        error_handler("Invalid item");
        return false;
    }
}
```

---

## Platform-Specific Tuning

### Linux x86_64 Optimizations

```cpp
#include <x86intrin.h>

// Use hardware prefetching
void prefetch_data(const void* addr) {
    _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0);
}

// SIMD optimizations for data processing
void simd_processing(turbokit::Vector<float>& data) {
    size_t simd_end = (data.size() / 8) * 8;

    for (size_t i = 0; i < simd_end; i += 8) {
        __m256 vec = _mm256_load_ps(&data[i]);
        vec = _mm256_mul_ps(vec, _mm256_set1_ps(2.0f));
        _mm256_store_ps(&data[i], vec);
    }

    // Handle remaining elements
    for (size_t i = simd_end; i < data.size(); ++i) {
        data[i] *= 2.0f;
    }
}
```

### CPU Feature Detection

```cpp
#include <cpuid.h>

class CPUFeatures {
private:
    bool has_avx2_ = false;
    bool has_bmi2_ = false;

public:
    CPUFeatures() {
        unsigned int eax, ebx, ecx, edx;

        // Check for AVX2
        if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
            has_avx2_ = (ebx & bit_AVX2) != 0;
            has_bmi2_ = (ebx & bit_BMI2) != 0;
        }

        turbokit::log.info("CPU Features: AVX2=%s, BMI2=%s",
                          has_avx2_ ? "yes" : "no",
                          has_bmi2_ ? "yes" : "no");
    }

    bool has_avx2() const { return has_avx2_; }
    bool has_bmi2() const { return has_bmi2_; }
};

void optimized_processing(turbokit::Vector<float>& data) {
    static CPUFeatures features;

    if (features.has_avx2()) {
        simd_processing(data);
    } else {
        scalar_processing(data);
    }
}
```

---

## Common Performance Pitfalls

### 1. Unnecessary Allocations

```cpp
// ❌ Bad: Repeated allocations
void process_messages() {
    for (int i = 0; i < 1000000; ++i) {
        turbokit::Vector<Item> temp_items;  // New allocation each iteration
        load_items(temp_items);
        process_items(temp_items);
    }
}

// ✅ Good: Reuse allocations
void process_messages() {
    turbokit::Vector<Item> temp_items;
    temp_items.reserve(1000);  // Pre-allocate

    for (int i = 0; i < 1000000; ++i) {
        temp_items.clear();     // Clear but keep memory
        load_items(temp_items);
        process_items(temp_items);
    }
}
```

### 2. False Sharing

```cpp
// ❌ Bad: False sharing between threads
struct BadCounters {
    std::atomic<uint64_t> counter1;
    std::atomic<uint64_t> counter2;  // Same cache line as counter1
};

// ✅ Good: Prevent false sharing
struct GoodCounters {
    alignas(std::hardware_destructive_interference_size) std::atomic<uint64_t> counter1;
    alignas(std::hardware_destructive_interference_size) std::atomic<uint64_t> counter2;
};
```

### 3. Inefficient String Operations

```cpp
// ❌ Bad: String concatenation in loops
std::string build_string() {
    std::string result;
    for (int i = 0; i < 1000; ++i) {
        result += std::to_string(i) + ",";  // Multiple allocations
    }
    return result;
}

// ✅ Good: Pre-allocate or use efficient building
std::string build_string() {
    std::string result;
    result.reserve(5000);  // Estimate final size

    for (int i = 0; i < 1000; ++i) {
        result += std::to_string(i);
        if (i < 999) result += ",";
    }
    return result;
}
```

### 4. Premature Hash Map Resizing

```cpp
// ❌ Bad: Let hash map grow organically
turbokit::HashMap<int, std::string> map;
for (int i = 0; i < 100000; ++i) {
    map[i] = std::to_string(i);  // Multiple rehashing operations
}

// ✅ Good: Pre-size the hash map
turbokit::HashMap<int, std::string> map;
map.reserve(100000);  // Single allocation
for (int i = 0; i < 100000; ++i) {
    map[i] = std::to_string(i);  // No rehashing
}
```

---

## Benchmarking Best Practices

### Robust Benchmarking Framework

```cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>

class RobustBenchmark {
private:
    const char* name_;
    size_t iterations_;
    size_t warmup_iterations_;
    turbokit::Vector<int64_t> measurements_;

public:
    RobustBenchmark(const char* name, size_t iterations = 1000, size_t warmup = 100)
        : name_(name), iterations_(iterations), warmup_iterations_(warmup) {
        measurements_.reserve(iterations);
    }

    template<typename Func>
    void run(Func&& func) {
        // Warm up
        for (size_t i = 0; i < warmup_iterations_; ++i) {
            func();
        }

        // Clear measurements
        measurements_.clear();

        // Run benchmarks
        for (size_t i = 0; i < iterations_; ++i) {
            auto start = turbokit::clock.get_current_time();
            func();
            auto end = turbokit::clock.get_current_time();
            measurements_.append(end - start);
        }

        analyze_results();
    }

private:
    void analyze_results() {
        if (measurements_.empty()) return;

        std::sort(measurements_.begin(), measurements_.end());

        size_t n = measurements_.size();
        int64_t min_val = measurements_[0];
        int64_t max_val = measurements_[n - 1];
        int64_t median = measurements_[n / 2];
        int64_t p95 = measurements_[n * 95 / 100];

        // Calculate mean and standard deviation
        double sum = 0;
        for (auto m : measurements_) sum += m;
        double mean = sum / n;

        double variance = 0;
        for (auto m : measurements_) {
            variance += (m - mean) * (m - mean);
        }
        double stddev = std::sqrt(variance / n);

        turbokit::log.info("Benchmark: %s", name_);
        turbokit::log.info("  Iterations: %zu", n);
        turbokit::log.info("  Min: %lld ns", min_val);
        turbokit::log.info("  Mean: %.2f ns (±%.2f)", mean, stddev);
        turbokit::log.info("  Median: %lld ns", median);
        turbokit::log.info("  P95: %lld ns", p95);
        turbokit::log.info("  Max: %lld ns", max_val);
        turbokit::log.info("  Coefficient of variation: %.2f%%", (stddev / mean) * 100);
    }
};

// Usage example
void benchmark_vector_operations() {
    constexpr size_t SIZE = 100000;

    {
        RobustBenchmark bench("Vector append (pre-allocated)");
        bench.run([&]() {
            turbokit::Vector<int> vec;
            vec.reserve(SIZE);
            for (size_t i = 0; i < SIZE; ++i) {
                vec.append(i);
            }
        });
    }

    {
        RobustBenchmark bench("Vector append (not pre-allocated)");
        bench.run([&]() {
            turbokit::Vector<int> vec;
            for (size_t i = 0; i < SIZE; ++i) {
                vec.append(i);
            }
        });
    }
}
```

### Environment Control

```cpp
#include <sched.h>
#include <sys/mman.h>

class BenchmarkEnvironment {
public:
    BenchmarkEnvironment() {
        // Lock memory to prevent paging
        if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
            turbokit::log.warning("Failed to lock memory");
        }

        // Set CPU affinity to reduce variance
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);  // Use CPU 0
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0) {
            turbokit::log.warning("Failed to set CPU affinity");
        }

        // Set high priority
        struct sched_param param;
        param.sched_priority = 99;
        if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
            turbokit::log.warning("Failed to set high priority");
        }

        turbokit::log.info("Benchmark environment initialized");
    }

    ~BenchmarkEnvironment() {
        munlockall();
    }
};

void run_controlled_benchmarks() {
    BenchmarkEnvironment env;  // Set up controlled environment

    // Run benchmarks with minimal system interference
    benchmark_vector_operations();
    benchmark_hashmap_operations();
    benchmark_clock_overhead();
}
```

---

## Performance Monitoring in Production

### Lightweight Metrics Collection

```cpp
#include <turbokit/hash_map.h>
#include <turbokit/sync.h>

class ProductionMetrics {
private:
    struct MetricData {
        std::atomic<uint64_t> count{0};
        std::atomic<uint64_t> total_time{0};
        std::atomic<uint64_t> max_time{0};
    };

    turbokit::HashMap<std::string, MetricData> metrics_;
    turbokit::SpinMutex metrics_mutex_;

public:
    void record_timing(const std::string& operation, uint64_t duration_ns) {
        std::lock_guard<turbokit::SpinMutex> lock(metrics_mutex_);
        auto& metric = metrics_[operation];

        metric.count.fetch_add(1, std::memory_order_relaxed);
        metric.total_time.fetch_add(duration_ns, std::memory_order_relaxed);

        // Update max atomically
        uint64_t current_max = metric.max_time.load(std::memory_order_relaxed);
        while (duration_ns > current_max) {
            if (metric.max_time.compare_exchange_weak(current_max, duration_ns,
                                                     std::memory_order_relaxed)) {
                break;
            }
        }
    }

    void print_summary() const {
        std::lock_guard<turbokit::SpinMutex> lock(metrics_mutex_);
        turbokit::log.info("=== Performance Summary ===");

        for (const auto& [operation, metric] : metrics_) {
            uint64_t count = metric.count.load(std::memory_order_relaxed);
            uint64_t total = metric.total_time.load(std::memory_order_relaxed);
            uint64_t max_time = metric.max_time.load(std::memory_order_relaxed);

            if (count > 0) {
                double avg = static_cast<double>(total) / count;
                turbokit::log.info("%s: count=%llu, avg=%.2f ns, max=%llu ns",
                                  operation.c_str(), count, avg, max_time);
            }
        }
    }
};

// Usage with RAII
class ScopedMetric {
private:
    ProductionMetrics& metrics_;
    std::string operation_;
    int64_t start_time_;

public:
    ScopedMetric(ProductionMetrics& metrics, std::string operation)
        : metrics_(metrics), operation_(std::move(operation)),
          start_time_(turbokit::clock.get_current_time()) {}

    ~ScopedMetric() {
        auto duration = turbokit::clock.get_current_time() - start_time_;
        metrics_.record_timing(operation_, duration);
    }
};

#define MEASURE_PERFORMANCE(metrics, name) \
    ScopedMetric _metric(metrics, name)
```

---

Remember: **Profile first, optimize second**. Always measure the actual performance impact of your optimizations on your specific workload and hardware configuration.

---

**See Also:**
- [API Reference](api/README.md) - Detailed component documentation
- [Examples](examples/README.md) - Practical usage examples
- [Clock API](api/clock.md) - High-precision timing details