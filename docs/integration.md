# TurboKit Integration Guide

Comprehensive guide for integrating TurboKit into existing projects and build systems.

## Table of Contents

- [Quick Integration](#quick-integration)
- [Build System Integration](#build-system-integration)
- [Migration Strategies](#migration-strategies)
- [Deployment Considerations](#deployment-considerations)
- [Testing and Validation](#testing-and-validation)
- [Monitoring and Profiling](#monitoring-and-profiling)
- [Common Integration Patterns](#common-integration-patterns)
- [Troubleshooting](#troubleshooting)

---

## Quick Integration

### Minimal Setup

For the fastest integration, TurboKit can be used as a header-only library:

```cpp
// Download headers to your project
git submodule add https://github.com/AlpinDale/turbokit.git third_party/turbokit

// Include in your source
#include "third_party/turbokit/include/turbokit/clock.h"
#include "third_party/turbokit/include/turbokit/vector.h"

int main() {
    auto start = turbokit::clock.get_current_time();
    turbokit::Vector<int> data;
    // ... your code ...
    auto end = turbokit::clock.get_current_time();
    return 0;
}
```

### CMake Integration (Recommended)

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Fetch TurboKit
include(FetchContent)
FetchContent_Declare(
    turbokit
    GIT_REPOSITORY https://github.com/AlpinDale/turbokit.git
    GIT_TAG main  # Or specific version tag
)
FetchContent_MakeAvailable(turbokit)

# Create your executable
add_executable(myapp main.cpp)

# Link TurboKit
target_link_libraries(myapp PRIVATE TurboKit::TurboKit)

# Set C++17 standard (required)
target_compile_features(myapp PRIVATE cxx_std_17)
```

---

## Build System Integration

### CMake Projects

#### Using FetchContent (Recommended)

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyHighPerformanceApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Optimization flags for performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Fetch TurboKit
include(FetchContent)
FetchContent_Declare(
    turbokit
    GIT_REPOSITORY https://github.com/AlpinDale/turbokit.git
    GIT_TAG v1.0.0  # Use specific version for production
    GIT_SHALLOW TRUE  # Faster download
)

# Configure TurboKit options
set(TURBOKIT_BUILD_TESTS OFF CACHE BOOL "Skip TurboKit tests")
set(TURBOKIT_BUILD_EXAMPLES OFF CACHE BOOL "Skip TurboKit examples")

FetchContent_MakeAvailable(turbokit)

# Your application
add_executable(myapp
    src/main.cpp
    src/performance_critical.cpp
    src/data_processing.cpp
)

target_link_libraries(myapp PRIVATE TurboKit::TurboKit)

# Optional: Enable additional warnings
target_compile_options(myapp PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
```

#### Using Submodules

```bash
# Add as submodule
git submodule add https://github.com/AlpinDale/turbokit.git third_party/turbokit
git submodule update --init --recursive
```

```cmake
# CMakeLists.txt
add_subdirectory(third_party/turbokit)
target_link_libraries(myapp PRIVATE TurboKit::TurboKit)
```

#### Using find_package (Installed Version)

```bash
# Install TurboKit system-wide
git clone https://github.com/AlpinDale/turbokit.git
cd turbokit
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make install
```

```cmake
# CMakeLists.txt
find_package(TurboKit REQUIRED)
target_link_libraries(myapp PRIVATE TurboKit::TurboKit)
```

### Bazel Integration

```python
# WORKSPACE
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "turbokit",
    remote = "https://github.com/AlpinDale/turbokit.git",
    tag = "v1.0.0",
)

load("@turbokit//bazel:deps.bzl", "turbokit_deps")
turbokit_deps()
```

```python
# BUILD
cc_binary(
    name = "myapp",
    srcs = ["main.cpp"],
    deps = ["@turbokit//:turbokit"],
    copts = ["-std=c++17", "-O3"],
)
```

### Meson Integration

```meson
# meson.build
project('myapp', 'cpp',
  version : '1.0.0',
  default_options : ['cpp_std=c++17', 'buildtype=release'])

# Get TurboKit dependency
turbokit_dep = dependency('turbokit', fallback : ['turbokit', 'turbokit_dep'])

# Build executable
executable('myapp',
  'src/main.cpp',
  dependencies : turbokit_dep,
  cpp_args : ['-march=native'])
```

### Makefile Integration

```makefile
# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -DNDEBUG
INCLUDES = -I./third_party/turbokit/include
LIBS = -lfmt

SOURCES = main.cpp performance_critical.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = myapp

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
```

---

## Migration Strategies

### Gradual Migration from STL

#### Phase 1: Drop-in Replacements

Replace performance-critical STL containers with TurboKit equivalents:

```cpp
// Before
#include <vector>
#include <unordered_map>

class DataProcessor {
private:
    std::vector<double> data_;
    std::unordered_map<std::string, int> lookup_;
public:
    void process() {
        for (auto& value : data_) {
            // Processing logic
        }
    }
};

// After (minimal changes)
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>

class DataProcessor {
private:
    turbokit::Vector<double> data_;
    turbokit::HashMap<std::string, int> lookup_;
public:
    void process() {
        for (auto& value : data_) {
            // Same processing logic
        }
    }
};
```

#### Phase 2: Optimize Memory Management

Add explicit capacity management:

```cpp
class OptimizedDataProcessor {
private:
    turbokit::Vector<double> data_;
    turbokit::HashMap<std::string, int> lookup_;

public:
    OptimizedDataProcessor(size_t expected_data_size, size_t expected_lookups) {
        // Pre-allocate for predictable performance
        data_.reserve(expected_data_size);
        lookup_.reserve(expected_lookups);
    }

    void process_batch(const std::vector<double>& input) {
        // Clear without deallocation
        data_.clear();

        // Process efficiently
        for (double value : input) {
            data_.append(process_value(value));
        }
    }
};
```

#### Phase 3: Add Performance Monitoring

```cpp
#include <turbokit/clock.h>
#include <turbokit/logging.h>

class MonitoredDataProcessor {
private:
    turbokit::Vector<double> data_;
    turbokit::HashMap<std::string, int> lookup_;

public:
    void process_batch(const std::vector<double>& input) {
        auto start = turbokit::clock.get_current_time();

        data_.clear();
        for (double value : input) {
            data_.append(process_value(value));
        }

        auto end = turbokit::clock.get_current_time();
        auto duration_us = (end - start) / 1000;

        turbokit::log.info("Processed %zu items in %lld μs",
                          input.size(), duration_us);
    }
};
```

### Migration from Custom Containers

#### Replacing Custom Vector

```cpp
// Before: Custom vector implementation
class CustomVector {
private:
    double* data_;
    size_t size_;
    size_t capacity_;

public:
    // Custom implementation...
};

// After: TurboKit Vector with same interface
class MigratedVector {
private:
    turbokit::Vector<double> impl_;

public:
    // Adapter methods for backward compatibility
    double* data() { return impl_.get_data(); }
    size_t size() const { return impl_.size(); }
    size_t capacity() const { return impl_.get_capacity(); }

    void push_back(double value) { impl_.append(value); }
    void reserve(size_t cap) { impl_.reserve(cap); }

    // Direct access to TurboKit implementation
    turbokit::Vector<double>& get_impl() { return impl_; }
};
```

#### Replacing Custom Hash Table

```cpp
// Before: Custom hash table
class CustomHashTable {
    // Custom implementation
};

// After: TurboKit HashMap with compatibility layer
template<typename K, typename V>
class MigratedHashTable {
private:
    turbokit::HashMap<K, V> impl_;

public:
    bool insert(const K& key, const V& value) {
        auto [it, inserted] = impl_.insert(key, value);
        return inserted;
    }

    V* find(const K& key) {
        auto it = impl_.find(key);
        return (it != impl_.end()) ? &it->second : nullptr;
    }

    void remove(const K& key) {
        impl_.erase(key);
    }

    // Gradual migration: expose TurboKit interface
    turbokit::HashMap<K, V>& get_impl() { return impl_; }
};
```

---

## Deployment Considerations

### Production Build Configuration

```cmake
# Production CMake configuration
cmake_minimum_required(VERSION 3.16)
project(ProductionApp)

# Strict C++17 requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Production optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")

# Enable interprocedural optimization
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# TurboKit configuration for production
set(TURBOKIT_BUILD_TESTS OFF)
set(TURBOKIT_BUILD_EXAMPLES OFF)
set(TURBOKIT_ENABLE_SANITIZERS OFF)

# Security hardening (optional)
if(ENABLE_HARDENING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
endif()

# Link TurboKit
include(FetchContent)
FetchContent_Declare(turbokit
    GIT_REPOSITORY https://github.com/AlpinDale/turbokit.git
    GIT_TAG v1.0.0)
FetchContent_MakeAvailable(turbokit)

add_executable(prodapp src/main.cpp)
target_link_libraries(prodapp PRIVATE TurboKit::TurboKit)
```

### Docker Integration

```dockerfile
# Dockerfile for TurboKit applications
FROM ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy source code
COPY . .

# Build with optimizations
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=native -flto" \
    && cmake --build build --parallel

# Production image
FROM ubuntu:22.04 as runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Copy binary
COPY --from=builder /build/build/myapp /usr/local/bin/

# Set up non-root user
RUN useradd -r -s /bin/false myapp
USER myapp

# Run application
CMD ["/usr/local/bin/myapp"]
```

### Packaging for Distribution

```cmake
# CPack configuration for packaging
include(CPack)

set(CPACK_PACKAGE_NAME "MyTurboKitApp")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "High-performance application using TurboKit")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")

# DEB package configuration
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libfmt-dev")

# RPM package configuration
set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++, fmt-devel")

# Create packages
# cmake --build build --target package
```

---

## Testing and Validation

### Unit Testing Integration

```cpp
// test_turbokit_integration.cpp
#include <gtest/gtest.h>
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>
#include <turbokit/clock.h>

class TurboKitIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize any test fixtures
    }
};

TEST_F(TurboKitIntegrationTest, VectorPerformance) {
    const size_t SIZE = 100000;

    auto start = turbokit::clock.get_current_time();

    turbokit::Vector<int> vec;
    vec.reserve(SIZE);

    for (size_t i = 0; i < SIZE; ++i) {
        vec.append(i);
    }

    auto end = turbokit::clock.get_current_time();
    auto duration_us = (end - start) / 1000;

    // Performance expectation: < 10ms for 100k elements
    EXPECT_LT(duration_us, 10000);
    EXPECT_EQ(vec.size(), SIZE);
}

TEST_F(TurboKitIntegrationTest, HashMapCorrectness) {
    turbokit::HashMap<std::string, int> map;

    // Test basic operations
    map["key1"] = 42;
    map["key2"] = 24;

    EXPECT_EQ(map["key1"], 42);
    EXPECT_EQ(map["key2"], 24);
    EXPECT_EQ(map.size(), 2);

    // Test find operation
    auto it = map.find("key1");
    EXPECT_NE(it, map.end());
    EXPECT_EQ(it->second, 42);
}

TEST_F(TurboKitIntegrationTest, ClockPrecision) {
    // Test clock monotonicity
    auto time1 = turbokit::clock.get_current_time();
    auto time2 = turbokit::clock.get_current_time();
    auto time3 = turbokit::clock.get_current_time();

    EXPECT_GE(time2, time1);
    EXPECT_GE(time3, time2);

    // Test precision (should be < 1μs overhead)
    auto overhead = time2 - time1;
    EXPECT_LT(overhead, 1000);  // < 1μs
}
```

### Performance Regression Testing

```cpp
// performance_regression_test.cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>
#include <fstream>

class PerformanceRegressionTest {
private:
    std::string baseline_file_;

public:
    PerformanceRegressionTest(const std::string& baseline)
        : baseline_file_(baseline) {}

    void test_vector_performance() {
        const size_t SIZE = 1000000;

        auto start = turbokit::clock.get_current_time();

        turbokit::Vector<int> vec;
        vec.reserve(SIZE);
        for (size_t i = 0; i < SIZE; ++i) {
            vec.append(i);
        }

        auto end = turbokit::clock.get_current_time();
        auto duration_ns = end - start;

        // Load baseline
        int64_t baseline = load_baseline("vector_append");

        // Check for regression (>10% slower)
        if (duration_ns > baseline * 1.1) {
            throw std::runtime_error("Performance regression detected");
        }

        // Update baseline if significantly faster
        if (duration_ns < baseline * 0.9) {
            save_baseline("vector_append", duration_ns);
        }
    }

private:
    int64_t load_baseline(const std::string& test_name) {
        std::ifstream file(baseline_file_);
        // Load baseline implementation...
        return 1000000; // Placeholder
    }

    void save_baseline(const std::string& test_name, int64_t duration) {
        std::ofstream file(baseline_file_, std::ios::app);
        // Save baseline implementation...
    }
};
```

### Memory Testing

```cpp
// memory_test.cpp
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>

#ifdef ENABLE_VALGRIND
#include <valgrind/memcheck.h>
#endif

class MemoryTest {
public:
    void test_no_leaks() {
        {
            turbokit::Vector<std::string> vec;
            vec.reserve(1000);

            for (int i = 0; i < 1000; ++i) {
                vec.append("test_string_" + std::to_string(i));
            }

            turbokit::HashMap<int, std::string> map;
            map.reserve(1000);

            for (int i = 0; i < 1000; ++i) {
                map[i] = "value_" + std::to_string(i);
            }
        } // All objects destroyed here

#ifdef ENABLE_VALGRIND
        // Check for memory leaks
        VALGRIND_DO_LEAK_CHECK;
#endif
    }
};
```

---

## Monitoring and Profiling

### Production Monitoring

```cpp
// production_monitor.cpp
#include <turbokit/clock.h>
#include <turbokit/hash_map.h>
#include <turbokit/logging.h>
#include <thread>
#include <atomic>

class ProductionMonitor {
private:
    std::atomic<bool> running_{true};
    std::thread monitor_thread_;

    struct PerformanceStats {
        std::atomic<uint64_t> operation_count{0};
        std::atomic<uint64_t> total_time{0};
        std::atomic<uint64_t> max_time{0};
    };

    turbokit::HashMap<std::string, PerformanceStats> stats_;

public:
    ProductionMonitor() : monitor_thread_(&ProductionMonitor::monitor_loop, this) {}

    ~ProductionMonitor() {
        running_ = false;
        monitor_thread_.join();
    }

    void record_operation(const std::string& op, uint64_t duration_ns) {
        auto& stat = stats_[op];
        stat.operation_count.fetch_add(1, std::memory_order_relaxed);
        stat.total_time.fetch_add(duration_ns, std::memory_order_relaxed);

        // Update max time atomically
        uint64_t current_max = stat.max_time.load(std::memory_order_relaxed);
        while (duration_ns > current_max) {
            if (stat.max_time.compare_exchange_weak(current_max, duration_ns,
                                                   std::memory_order_relaxed)) {
                break;
            }
        }
    }

private:
    void monitor_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(60));

            turbokit::log.info("=== Performance Report ===");
            for (const auto& [operation, stat] : stats_) {
                uint64_t count = stat.operation_count.load();
                uint64_t total = stat.total_time.load();
                uint64_t max_time = stat.max_time.load();

                if (count > 0) {
                    double avg_us = (total / 1000.0) / count;
                    double max_us = max_time / 1000.0;

                    turbokit::log.info("%s: %llu ops, avg=%.2f μs, max=%.2f μs",
                                      operation.c_str(), count, avg_us, max_us);
                }
            }
        }
    }
};

// Global monitor instance
ProductionMonitor g_monitor;

// RAII helper for automatic timing
class ScopedTimer {
private:
    std::string operation_;
    int64_t start_time_;

public:
    ScopedTimer(std::string operation)
        : operation_(std::move(operation)),
          start_time_(turbokit::clock.get_current_time()) {}

    ~ScopedTimer() {
        auto duration = turbokit::clock.get_current_time() - start_time_;
        g_monitor.record_operation(operation_, duration);
    }
};

#define MONITOR_PERFORMANCE(op) ScopedTimer _timer(op)
```

### Integration with External Profilers

```cpp
// profiler_integration.cpp
#ifdef ENABLE_PROFILER
#include <gperftools/profiler.h>
#endif

class ProfilerIntegration {
public:
    static void start_profiling(const std::string& filename) {
#ifdef ENABLE_PROFILER
        ProfilerStart(filename.c_str());
        turbokit::log.info("Started profiling to %s", filename.c_str());
#endif
    }

    static void stop_profiling() {
#ifdef ENABLE_PROFILER
        ProfilerStop();
        turbokit::log.info("Stopped profiling");
#endif
    }

    static void flush_profiler() {
#ifdef ENABLE_PROFILER
        ProfilerFlush();
#endif
    }
};

// Usage in main application
int main() {
    ProfilerIntegration::start_profiling("myapp_profile.prof");

    // Your application code here
    run_performance_critical_code();

    ProfilerIntegration::stop_profiling();
    return 0;
}
```

---

## Common Integration Patterns

### Dependency Injection

```cpp
// dependency_injection.cpp
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>
#include <memory>

// Abstract interfaces
class IDataContainer {
public:
    virtual ~IDataContainer() = default;
    virtual void add_item(int value) = 0;
    virtual size_t size() const = 0;
};

class ILookupTable {
public:
    virtual ~ILookupTable() = default;
    virtual void insert(const std::string& key, int value) = 0;
    virtual bool find(const std::string& key, int& value) const = 0;
};

// TurboKit implementations
class TurboKitDataContainer : public IDataContainer {
private:
    turbokit::Vector<int> data_;

public:
    TurboKitDataContainer(size_t expected_size = 1000) {
        data_.reserve(expected_size);
    }

    void add_item(int value) override {
        data_.append(value);
    }

    size_t size() const override {
        return data_.size();
    }
};

class TurboKitLookupTable : public ILookupTable {
private:
    turbokit::HashMap<std::string, int> table_;

public:
    TurboKitLookupTable(size_t expected_size = 1000) {
        table_.reserve(expected_size);
    }

    void insert(const std::string& key, int value) override {
        table_[key] = value;
    }

    bool find(const std::string& key, int& value) const override {
        auto it = table_.find(key);
        if (it != table_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }
};

// Application using dependency injection
class Application {
private:
    std::unique_ptr<IDataContainer> container_;
    std::unique_ptr<ILookupTable> lookup_;

public:
    Application(std::unique_ptr<IDataContainer> container,
               std::unique_ptr<ILookupTable> lookup)
        : container_(std::move(container)), lookup_(std::move(lookup)) {}

    void run() {
        // Use injected dependencies
        container_->add_item(42);
        lookup_->insert("key", 123);
    }
};

// Factory function
std::unique_ptr<Application> create_turbokit_app() {
    auto container = std::make_unique<TurboKitDataContainer>(10000);
    auto lookup = std::make_unique<TurboKitLookupTable>(1000);
    return std::make_unique<Application>(std::move(container), std::move(lookup));
}
```

### Plugin Architecture

```cpp
// plugin_architecture.cpp
#include <turbokit/vector.h>
#include <turbokit/hash_map.h>

// Plugin interface
class IPerformancePlugin {
public:
    virtual ~IPerformancePlugin() = default;
    virtual void initialize() = 0;
    virtual void process_data(const turbokit::Vector<double>& input,
                             turbokit::Vector<double>& output) = 0;
    virtual const char* get_name() const = 0;
};

// TurboKit-based plugin implementation
class FastMathPlugin : public IPerformancePlugin {
private:
    turbokit::Vector<double> temp_buffer_;

public:
    void initialize() override {
        temp_buffer_.reserve(10000);  // Pre-allocate working space
    }

    void process_data(const turbokit::Vector<double>& input,
                     turbokit::Vector<double>& output) override {
        output.clear();
        output.reserve(input.size());

        for (double value : input) {
            output.append(std::sqrt(value * value + 1.0));
        }
    }

    const char* get_name() const override {
        return "FastMathPlugin";
    }
};

// Plugin manager
class PluginManager {
private:
    turbokit::HashMap<std::string, std::unique_ptr<IPerformancePlugin>> plugins_;

public:
    void register_plugin(std::unique_ptr<IPerformancePlugin> plugin) {
        std::string name = plugin->get_name();
        plugin->initialize();
        plugins_[name] = std::move(plugin);
    }

    IPerformancePlugin* get_plugin(const std::string& name) {
        auto it = plugins_.find(name);
        return (it != plugins_.end()) ? it->second.get() : nullptr;
    }
};
```

### Configuration Management

```cpp
// config_management.cpp
#include <turbokit/hash_map.h>
#include <turbokit/logging.h>
#include <fstream>
#include <sstream>

class TurboKitConfig {
private:
    turbokit::HashMap<std::string, std::string> config_values_;

public:
    void load_from_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            turbokit::log.error("Failed to open config file: %s", filename.c_str());
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                config_values_[key] = value;
            }
        }

        turbokit::log.info("Loaded %zu configuration values", config_values_.size());
    }

    template<typename T>
    T get_value(const std::string& key, const T& default_value) const {
        auto it = config_values_.find(key);
        if (it != config_values_.end()) {
            return parse_value<T>(it->second);
        }
        return default_value;
    }

    void set_value(const std::string& key, const std::string& value) {
        config_values_[key] = value;
    }

private:
    template<typename T>
    T parse_value(const std::string& str) const {
        std::istringstream iss(str);
        T value;
        iss >> value;
        return value;
    }
};

// Usage
TurboKitConfig config;
config.load_from_file("app.conf");

size_t buffer_size = config.get_value("buffer_size", size_t(65536));
double timeout = config.get_value("timeout_seconds", 30.0);
```

---

## Troubleshooting

### Common Build Issues

#### Missing C++17 Support

```bash
# Error: TurboKit requires C++17
# Solution: Update compiler or CMake configuration

# For GCC < 7.0 or Clang < 6.0
sudo apt-get update
sudo apt-get install gcc-9 g++-9
export CXX=g++-9

# CMake configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

#### Missing fmt Dependency

```bash
# Error: fmt library not found
# Solution: Install fmt or let TurboKit fetch it

# Manual installation (Ubuntu/Debian)
sudo apt-get install libfmt-dev

# Or let TurboKit handle it automatically (recommended)
# FetchContent will download fmt automatically
```

#### TSC Not Available

```cpp
// Issue: Clock not working on virtualized environments
// Solution: Fallback detection and logging

#include <turbokit/clock.h>

void check_clock_functionality() {
    auto time1 = turbokit::clock.get_current_time();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto time2 = turbokit::clock.get_current_time();

    auto duration = time2 - time1;
    if (duration < 50000 || duration > 200000) {  // Outside 50-200μs range
        turbokit::log.warning("Clock may not be functioning correctly");
        turbokit::log.warning("Duration: %lld ns (expected ~100μs)", duration);
    }
}
```

### Runtime Issues

#### Performance Degradation

```cpp
// Diagnostic code for performance issues
#include <turbokit/clock.h>

class PerformanceDiagnostics {
public:
    static void diagnose_vector_performance() {
        const size_t SIZE = 100000;

        // Test with pre-allocation
        auto start1 = turbokit::clock.get_current_time();
        turbokit::Vector<int> vec1;
        vec1.reserve(SIZE);
        for (size_t i = 0; i < SIZE; ++i) {
            vec1.append(i);
        }
        auto end1 = turbokit::clock.get_current_time();

        // Test without pre-allocation
        auto start2 = turbokit::clock.get_current_time();
        turbokit::Vector<int> vec2;
        for (size_t i = 0; i < SIZE; ++i) {
            vec2.append(i);
        }
        auto end2 = turbokit::clock.get_current_time();

        turbokit::log.info("Vector performance:");
        turbokit::log.info("  With pre-allocation: %lld ns", end1 - start1);
        turbokit::log.info("  Without pre-allocation: %lld ns", end2 - start2);
        turbokit::log.info("  Speedup: %.2fx",
                          double(end2 - start2) / double(end1 - start1));
    }

    static void diagnose_hashmap_performance() {
        const size_t SIZE = 10000;

        turbokit::HashMap<int, int> map;
        map.reserve(SIZE);

        // Test insertion performance
        auto start = turbokit::clock.get_current_time();
        for (size_t i = 0; i < SIZE; ++i) {
            map[i] = i * 2;
        }
        auto mid = turbokit::clock.get_current_time();

        // Test lookup performance
        int sum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            sum += map[i];
        }
        auto end = turbokit::clock.get_current_time();

        turbokit::log.info("HashMap performance:");
        turbokit::log.info("  Insertion: %lld ns total, %.2f ns/op",
                          mid - start, double(mid - start) / SIZE);
        turbokit::log.info("  Lookup: %lld ns total, %.2f ns/op",
                          end - mid, double(end - mid) / SIZE);
        turbokit::log.info("  Load factor: %.2f", map.load_factor());
    }
};
```

#### Memory Issues

```cpp
// Memory debugging utilities
#include <turbokit/vector.h>

class MemoryDiagnostics {
public:
    static void check_memory_usage() {
        // Monitor vector memory usage
        turbokit::Vector<int> vec;

        turbokit::log.info("Initial: size=%zu, capacity=%zu",
                          vec.size(), vec.get_capacity());

        vec.reserve(1000);
        turbokit::log.info("After reserve(1000): size=%zu, capacity=%zu",
                          vec.size(), vec.get_capacity());

        for (int i = 0; i < 500; ++i) {
            vec.append(i);
        }
        turbokit::log.info("After 500 appends: size=%zu, capacity=%zu",
                          vec.size(), vec.get_capacity());

        vec.clear();
        turbokit::log.info("After clear(): size=%zu, capacity=%zu",
                          vec.size(), vec.get_capacity());

        vec.reset();
        turbokit::log.info("After reset(): size=%zu, capacity=%zu",
                          vec.size(), vec.get_capacity());
    }
};
```

### Getting Help

#### Enable Debug Logging

```cpp
// Enable verbose logging for troubleshooting
turbokit::currentLogLevel = turbokit::LOG_DEBUG;

// Your code here - all operations will be logged
```

#### Create Minimal Reproduction

```cpp
// minimal_repro.cpp - Template for bug reports
#include <turbokit/clock.h>
#include <turbokit/vector.h>
#include <turbokit/logging.h>

int main() {
    turbokit::log.info("TurboKit minimal reproduction case");

    // Minimal code that reproduces the issue
    turbokit::Vector<int> vec;
    vec.append(42);

    turbokit::log.info("Vector size: %zu", vec.size());
    turbokit::log.info("Vector[0]: %d", vec[0]);

    return 0;
}
```

For additional support:
- **GitHub Issues**: https://github.com/AlpinDale/turbokit/issues
- **Discussions**: https://github.com/AlpinDale/turbokit/discussions
- **Documentation**: Full API reference and examples

---

**Next Steps:**
- Review the [Performance Guide](performance.md) for optimization techniques
- Explore [Examples](examples/README.md) for practical usage patterns
- Check the [API Reference](api/README.md) for detailed documentation