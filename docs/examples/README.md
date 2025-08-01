# TurboKit Examples and Tutorials

Comprehensive examples demonstrating TurboKit's features and best practices.

## Table of Contents

### Getting Started
- [Quick Start Guide](#quick-start-guide)
- [Basic Usage Examples](#basic-usage-examples)
- [Common Patterns](#common-patterns)

### Component Examples
- [High-Precision Timing](timing.md) - Clock API examples
- [Container Operations](containers.md) - Vector and HashMap usage
- [Serialization Guide](serialization.md) - Binary serialization examples
- [Synchronization Patterns](synchronization.md) - Thread-safe programming
- [Memory Management](memory.md) - Buffer and FreeList usage
- [Logging Best Practices](logging.md) - Performance-oriented logging

### Advanced Topics
- [Performance Optimization](performance_optimization.md) - Maximizing throughput
- [Custom Allocators](custom_allocators.md) - Memory management customization
- [Real-Time Systems](realtime.md) - Deterministic performance patterns
- [Integration Patterns](integration.md) - Using TurboKit with existing code

### Complete Applications
- [High-Frequency Trading System](hft_example.md) - Low-latency financial system
- [Game Engine Components](game_engine.md) - Real-time game programming
- [Data Processing Pipeline](data_pipeline.md) - High-throughput data processing
- [Network Server](network_server.md) - High-performance server implementation

---

## Quick Start Guide

### Installation and Setup

```cpp
// CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
    turbokit
    GIT_REPOSITORY https://github.com/AlpinDale/turbokit.git
    GIT_TAG main
)
FetchContent_MakeAvailable(turbokit)
target_link_libraries(your_project PRIVATE TurboKit::TurboKit)
```

```cpp
// main.cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>
#include <turbokit/logging.h>

int main() {
    turbokit::log.info("TurboKit application starting");

    // Your high-performance code here

    return 0;
}
```

### Your First TurboKit Program

```cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>
#include <turbokit/logging.h>

int main() {
    // High-precision timing
    auto start = turbokit::clock.get_current_time();

    // High-performance container
    turbokit::Vector<int> numbers;
    numbers.reserve(1000000); // Pre-allocate for performance

    // Fill with data
    for (int i = 0; i < 1000000; ++i) {
        numbers.append(i * i);
    }

    // Measure performance
    auto end = turbokit::clock.get_current_time();
    auto duration_us = (end - start) / 1000;

    // Performance logging
    turbokit::log.info("Processed %zu numbers in %lld μs",
                       numbers.size(), duration_us);

    return 0;
}
```

---

## Basic Usage Examples

### 1. Timing and Profiling

```cpp
#include <turbokit/clock.h>
#include <turbokit/logging.h>

class PerformanceTimer {
private:
    int64_t start_time;
    const char* operation_name;

public:
    PerformanceTimer(const char* name)
        : operation_name(name), start_time(turbokit::clock.get_current_time()) {}

    ~PerformanceTimer() {
        auto end_time = turbokit::clock.get_current_time();
        auto duration_ns = end_time - start_time;
        turbokit::log.info("%s took %lld ns", operation_name, duration_ns);
    }
};

void example_timing() {
    {
        PerformanceTimer timer("Database query");
        // Simulate database operation
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
        PerformanceTimer timer("Data processing");
        // Simulate processing
        for (int i = 0; i < 1000000; ++i) {
            volatile int x = i * i; // Prevent optimization
        }
    }
}
```

### 2. High-Performance Data Structures

```cpp
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>

void example_containers() {
    // Pre-allocated vector for predictable performance
    turbokit::Vector<double> sensor_data;
    sensor_data.reserve(100000);

    // High-performance hash map for lookups
    turbokit::HashMap<std::string, int> device_ids;
    device_ids.reserve(1000);

    // Fill containers efficiently
    for (int i = 0; i < 100000; ++i) {
        sensor_data.append(read_sensor_value(i));

        if (i < 1000) {
            device_ids["device_" + std::to_string(i)] = i;
        }
    }

    // Fast lookups
    auto device_it = device_ids.find("device_42");
    if (device_it != device_ids.end()) {
        int device_id = device_it->second;
        double value = sensor_data[device_id];
        turbokit::log.info("Device 42 value: %f", value);
    }
}
```

### 3. Memory Management

```cpp
#include <turbokit/buffer.h>
#include <turbokit/freelist.h>

struct Message {
    int64_t timestamp;
    uint32_t type;
    uint32_t size;
    // Intrusive pointer for FreeList
    Message* next;
};

void example_memory_management() {
    // Efficient buffer management
    auto buffer = turbokit::Buffer::create(65536); // 64KB buffer
    turbokit::BufferHandle handle(buffer);

    // Fill buffer with data
    auto* data = handle->get_data();
    std::memcpy(data, "Hello, TurboKit!", 16);

    // Object pooling with FreeList
    using MessagePool = turbokit::FreeList<Message>;

    // Allocate from pool (very fast)
    Message* msg = MessagePool::remove_element();
    if (!msg) {
        msg = new Message; // Fallback allocation
    }

    // Use message
    msg->timestamp = turbokit::clock.get_current_time();
    msg->type = 1;
    msg->size = 100;

    // Return to pool (very fast)
    MessagePool::add_element(msg, 1000); // Max 1000 in local pool
}
```

### 4. Serialization

```cpp
#include <turbokit/serialization.h>
#include <turbokit/buffer.h>

struct UserProfile {
    uint64_t user_id;
    std::string username;
    std::vector<std::string> interests;
    double score;

    // Serialization support
    template<typename Context>
    void serialize(Context& ctx) {
        ctx(user_id, username, interests, score);
    }
};

void example_serialization() {
    // Create sample data
    UserProfile profile{
        .user_id = 12345,
        .username = "alice_wonderland",
        .interests = {"reading", "hiking", "programming"},
        .score = 95.7
    };

    // Serialize to buffer
    auto buffer = turbokit::serializeToBuffer(profile);
    turbokit::log.info("Serialized %zu bytes", buffer->get_size());

    // Deserialize back
    UserProfile restored_profile;
    turbokit::deserializeBuffer(buffer, restored_profile);

    // Verify data integrity
    assert(profile.user_id == restored_profile.user_id);
    assert(profile.username == restored_profile.username);
    assert(profile.interests == restored_profile.interests);
    assert(profile.score == restored_profile.score);

    turbokit::log.info("Serialization roundtrip successful");
}
```

---

## Common Patterns

### 1. RAII Resource Management

```cpp
#include <turbokit/buffer.h>
#include <turbokit/clock.h>

class ScopedBuffer {
private:
    turbokit::BufferHandle buffer_;

public:
    ScopedBuffer(size_t size) : buffer_(turbokit::Buffer::create(size)) {}

    std::byte* data() { return buffer_->get_data(); }
    size_t size() const { return buffer_->get_size(); }

    // Automatic cleanup on destruction
};

class ScopedTimer {
private:
    int64_t start_time_;
    std::string operation_;

public:
    ScopedTimer(std::string operation)
        : operation_(std::move(operation)), start_time_(turbokit::clock.get_current_time()) {}

    ~ScopedTimer() {
        auto duration = turbokit::clock.get_current_time() - start_time_;
        turbokit::log.info("%s: %lld ns", operation_.c_str(), duration);
    }
};

void example_raii() {
    ScopedTimer timer("Data processing");
    ScopedBuffer buffer(1024);

    // Use buffer safely - automatic cleanup on scope exit
    std::memset(buffer.data(), 0, buffer.size());

    // Timer automatically logs duration on destruction
}
```

### 2. High-Performance Event Processing

```cpp
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>
#include <turbokit/clock.h>

struct Event {
    uint64_t timestamp;
    uint32_t type;
    uint32_t source_id;
    uint64_t data;
};

class EventProcessor {
private:
    turbokit::Vector<Event> event_queue_;
    turbokit::HashMap<uint32_t, uint64_t> source_counters_;

public:
    EventProcessor() {
        // Pre-allocate for performance
        event_queue_.reserve(1000000);
        source_counters_.reserve(10000);
    }

    void add_event(const Event& event) {
        event_queue_.append(event);
        source_counters_[event.source_id]++;
    }

    void process_batch() {
        auto start = turbokit::clock.get_current_time();

        // Process all queued events
        for (const auto& event : event_queue_) {
            process_single_event(event);
        }

        // Clear queue efficiently (no deallocation)
        event_queue_.clear();

        auto duration = turbokit::clock.get_current_time() - start;
        turbokit::log.info("Processed batch in %lld ns", duration);
    }

private:
    void process_single_event(const Event& event) {
        // Event processing logic
        switch (event.type) {
            case 1: handle_type1_event(event); break;
            case 2: handle_type2_event(event); break;
            default: handle_unknown_event(event); break;
        }
    }
};
```

### 3. Thread-Safe Caching

```cpp
#include <turbokit/hash_map.h>
#include <turbokit/sync.h>
#include <shared_mutex>

template<typename Key, typename Value>
class ThreadSafeCache {
private:
    mutable std::shared_mutex mutex_;
    turbokit::HashMap<Key, Value> cache_;

public:
    ThreadSafeCache(size_t expected_size = 1000) {
        cache_.reserve(expected_size);
    }

    bool get(const Key& key, Value& value) const {
        std::shared_lock lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    void put(const Key& key, const Value& value) {
        std::unique_lock lock(mutex_);
        cache_[key] = value;
    }

    bool contains(const Key& key) const {
        std::shared_lock lock(mutex_);
        return cache_.find(key) != cache_.end();
    }

    void clear() {
        std::unique_lock lock(mutex_);
        cache_.clear();
    }

    size_t size() const {
        std::shared_lock lock(mutex_);
        return cache_.size();
    }
};

void example_thread_safe_cache() {
    ThreadSafeCache<std::string, int> cache(10000);

    // Multiple threads can safely access
    std::vector<std::thread> threads;

    // Reader threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&cache, i]() {
            for (int j = 0; j < 1000; ++j) {
                std::string key = "key_" + std::to_string(j);
                int value;
                if (cache.get(key, value)) {
                    // Process cached value
                }
            }
        });
    }

    // Writer thread
    threads.emplace_back([&cache]() {
        for (int i = 0; i < 1000; ++i) {
            std::string key = "key_" + std::to_string(i);
            cache.put(key, i * i);
        }
    });

    for (auto& thread : threads) {
        thread.join();
    }
}
```

### 4. Performance Monitoring

```cpp
#include <turbokit/clock.h>
#include <turbokit/hash_map.h>
#include <turbokit/logging.h>

class PerformanceMonitor {
private:
    struct Metrics {
        uint64_t count = 0;
        int64_t total_time = 0;
        int64_t min_time = std::numeric_limits<int64_t>::max();
        int64_t max_time = 0;

        void add_sample(int64_t duration) {
            count++;
            total_time += duration;
            min_time = std::min(min_time, duration);
            max_time = std::max(max_time, duration);
        }

        double average() const {
            return count > 0 ? static_cast<double>(total_time) / count : 0.0;
        }
    };

    turbokit::HashMap<std::string, Metrics> metrics_;

public:
    class ScopedMeasurement {
    private:
        PerformanceMonitor& monitor_;
        std::string operation_;
        int64_t start_time_;

    public:
        ScopedMeasurement(PerformanceMonitor& monitor, std::string operation)
            : monitor_(monitor), operation_(std::move(operation)),
              start_time_(turbokit::clock.get_current_time()) {}

        ~ScopedMeasurement() {
            auto duration = turbokit::clock.get_current_time() - start_time_;
            monitor_.record(operation_, duration);
        }
    };

    void record(const std::string& operation, int64_t duration_ns) {
        metrics_[operation].add_sample(duration_ns);
    }

    ScopedMeasurement measure(const std::string& operation) {
        return ScopedMeasurement(*this, operation);
    }

    void print_report() const {
        turbokit::log.info("=== Performance Report ===");
        for (const auto& [operation, metrics] : metrics_) {
            turbokit::log.info("%s: count=%llu, avg=%.2f ns, min=%lld ns, max=%lld ns",
                              operation.c_str(), metrics.count, metrics.average(),
                              metrics.min_time, metrics.max_time);
        }
    }

    void reset() {
        metrics_.clear();
    }
};

void example_performance_monitoring() {
    PerformanceMonitor monitor;

    // Measure different operations
    for (int i = 0; i < 1000; ++i) {
        {
            auto measurement = monitor.measure("fast_operation");
            fast_operation();
        }

        {
            auto measurement = monitor.measure("slow_operation");
            slow_operation();
        }
    }

    // Print performance summary
    monitor.print_report();
}
```

---

## Integration Examples

### 1. Drop-in Replacement for STL

```cpp
// Before: Using STL containers
#include <vector>
#include <unordered_map>
#include <chrono>

std::vector<int> data;
std::unordered_map<std::string, int> lookup;
auto start = std::chrono::high_resolution_clock::now();

// After: Using TurboKit (minimal changes)
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>
#include <turbokit/clock.h>

turbokit::Vector<int> data;
turbokit::HashMap<std::string, int> lookup;
auto start = turbokit::clock.get_current_time();
```

### 2. Custom Allocator Integration

```cpp
#include <turbokit/vector.h>
#include <memory_resource>

void example_custom_allocator() {
    // Pool allocator for better performance
    std::array<std::byte, 65536> buffer;
    std::pmr::monotonic_buffer_resource pool(buffer.data(), buffer.size());
    std::pmr::polymorphic_allocator<int> alloc(&pool);

    // Use custom allocator with TurboKit
    turbokit::DynamicArray<int, std::pmr::polymorphic_allocator<int>> vec;
    vec.reserve(1000);

    for (int i = 0; i < 1000; ++i) {
        vec.append(i);
    }

    // All allocations came from the pool - very fast!
}
```

### 3. Integration with Existing Profilers

```cpp
#include <turbokit/clock.h>

#ifdef USE_EXTERNAL_PROFILER
#include "external_profiler.h"
#define PROFILE_SCOPE(name) EXTERNAL_PROFILE_SCOPE(name)
#else
#define PROFILE_SCOPE(name) ScopedTimer timer(name)
#endif

class ScopedTimer {
    int64_t start_;
    const char* name_;
public:
    ScopedTimer(const char* name) : name_(name), start_(turbokit::clock.get_current_time()) {}
    ~ScopedTimer() {
        auto duration = turbokit::clock.get_current_time() - start_;
        turbokit::log.debug("%s: %lld ns", name_, duration);
    }
};

void some_function() {
    PROFILE_SCOPE("some_function");
    // Function implementation
}
```

---

## Next Steps

- Explore specific component examples in the subdirectories
- Read the [Performance Guide](../performance.md) for optimization techniques
- Check out the [API Reference](../api/README.md) for detailed documentation
- Try the [complete application examples](hft_example.md) for real-world usage patterns

---