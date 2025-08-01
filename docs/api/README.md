# TurboKit API Reference

Complete API documentation for all TurboKit components.

## Table of Contents

- [Clock API](clock.md) - High-precision timing
- [Vector API](vector.md) - Optimized dynamic arrays
- [HashMap API](hashmap.md) - High-performance hash tables
- [Serialization API](serialization.md) - Fast binary serialization
- [Sync API](sync.md) - Low-level synchronization primitives
- [FreeList API](freelist.md) - Thread-local object pooling
- [Logging API](logging.md) - Performance-optimized logging
- [Buffer API](buffer.md) - Memory management utilities
- [IntrusiveList API](intrusive_list.md) - Intrusive data structures
- [SimpleVector API](simple_vector.md) - Lightweight vector implementation

## API Design Principles

### 1. Zero-Cost Abstractions
All TurboKit APIs are designed to have zero runtime overhead when used correctly. Template metaprogramming and compiler optimization ensure that high-level interfaces compile down to optimal machine code.

### 2. Explicit Resource Management
TurboKit favors explicit resource management over hidden allocations. This provides predictable performance characteristics and better control over memory usage.

### 3. Thread Safety
Components are designed with clear thread safety guarantees:
- **Thread-safe**: Safe for concurrent access from multiple threads
- **Thread-local**: Optimized for single-thread access with thread-local storage
- **External synchronization**: Requires external synchronization for concurrent access

### 4. STL Compatibility
Where possible, TurboKit maintains STL-compatible interfaces to allow drop-in replacement and integration with existing C++ code.

### 5. Performance First
API design prioritizes performance over convenience. This means:
- Explicit capacity management
- Manual memory management options
- Direct access to underlying data structures
- Minimal runtime checks (debug builds have additional validation)

## Performance Guidelines

### Memory Access Patterns
- Use sequential access patterns when possible
- Prefer batch operations over single-element operations
- Reserve capacity upfront to avoid reallocations

### Compilation
- Use Release builds for production (-O3 -DNDEBUG)
- Enable target-specific optimizations (-march=native)
- Use LTO (Link Time Optimization) for maximum performance

### Profiling
- Use TurboKit's Clock for micro-benchmarking
- Profile with realistic data sizes and access patterns
- Measure cache misses and memory bandwidth utilization

## Error Handling

TurboKit uses a mix of error handling strategies:

1. **Exceptions**: For resource allocation failures and invalid operations
2. **Return codes**: For optional operations that may fail
3. **Assertions**: For debug-time validation (disabled in release builds)
4. **Undefined behavior**: For performance-critical paths (documented)

## Namespace Organization

All TurboKit APIs are in the `turbokit` namespace:

```cpp
namespace turbokit {
    // Core timing functionality
    class Clock;

    // Container types
    template<typename T> class Vector;
    template<typename K, typename V> class HashMap;

    // Synchronization primitives
    class SpinMutex;

    // Memory management
    template<typename T> class FreeList;
    class Buffer;

    // Utilities
    class Logger;
}
```

## Build Configuration

### Preprocessor Macros

- `TURBOKIT_DEBUG`: Enable debug assertions and additional validation
- `TURBOKIT_NO_EXCEPTIONS`: Disable exception handling (embedded systems)
- `TURBOKIT_CUSTOM_ALLOCATOR`: Use custom allocator implementations

### CMake Options

- `TURBOKIT_BUILD_TESTS`: Build unit tests and benchmarks
- `TURBOKIT_BUILD_EXAMPLES`: Build example applications
- `TURBOKIT_ENABLE_SANITIZERS`: Enable AddressSanitizer and UBSan

## Version Compatibility

TurboKit follows semantic versioning (SemVer):

- **Major version**: Breaking API changes
- **Minor version**: New features, backward compatible
- **Patch version**: Bug fixes, performance improvements

Current version: **1.0.0**

## Migration Guide

### From STL Containers

```cpp
// STL
std::vector<int> vec;
vec.push_back(42);
vec.reserve(1000);

// TurboKit
turbokit::Vector<int> vec;
vec.append(42);
vec.reserve(1000);
```

### From Custom Hash Tables

```cpp
// Custom hash table
std::unordered_map<std::string, int> map;
map["key"] = 42;

// TurboKit
turbokit::HashMap<std::string, int> map;
map.insert("key", 42);
```

### From std::chrono

```cpp
// std::chrono
auto start = std::chrono::high_resolution_clock::now();
// ... work ...
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

// TurboKit
auto start = turbokit::clock.get_current_time();
// ... work ...
auto end = turbokit::clock.get_current_time();
auto duration_ns = end - start;
```

## Platform Support

### Primary Platform
- **Linux x86_64**: Full support with all optimizations
- **GCC 7+, Clang 6+**: Recommended compilers

### Experimental Support
- **macOS x86_64**: Basic functionality, reduced performance
- **Windows x86_64**: Limited testing, compatibility issues possible
- **ARM64**: Basic functionality, no platform-specific optimizations

### Required CPU Features
- **TSC (Time Stamp Counter)**: For optimal Clock performance
- **CMPXCHG16B**: For 128-bit atomic operations
- **SSE2**: For vectorized operations (automatically detected)

---

*For detailed documentation of each component, see the individual API reference pages.*