# TurboKit Documentation

**High-performance C++ utilities for when the standard library isn't fast enough.**

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Core Components](#core-components)
- [API Reference](#api-reference)
- [Examples & Tutorials](#examples--tutorials)
- [Performance Guide](#performance-guide)
- [Integration Guide](#integration-guide)
- [Contributing](#contributing)

---

## Overview

TurboKit is a header-only, high-performance C++ utilities library designed for performance-critical applications where every nanosecond matters. Built for systems that demand:

- **Sub-microsecond timing precision**
- **Lock-free algorithms where possible**
- **Cache-friendly data structures**
- **Zero runtime overhead**

Note: this documentation was written in collaboration with Claude 4 Sonnet.

### Key Features

- **Header-only**: No compilation required, just include headers
- **Zero dependencies**: Except for fmt library for formatting
- **Battle-tested**: Used in production systems
- **C++17 compatible**: Modern C++ with backward compatibility
- **Platform optimized**: Leverages x86 TSC, futex primitives, and compiler intrinsics

### Performance Philosophy

TurboKit is built on three core principles:

1. **Predictable Performance**: Every operation has deterministic time complexity
2. **Cache Efficiency**: Data structures are designed for spatial and temporal locality
3. **Minimal Overhead**: Zero-cost abstractions with compile-time optimizations

---

## Quick Start

### Basic Usage

```cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>
#include <turbokit/logging.h>

int main() {
    // High-precision timing
    auto start = turbokit::clock.get_current_time();

    // Optimized vector operations
    turbokit::Vector<int> data;
    data.append(42);
    data.append(24);

    auto duration = turbokit::clock.get_current_time() - start;

    // Thread-safe logging
    turbokit::log.info("Processed %zu items in %lld ns",
                       data.size(), duration);

    return 0;
}
```

### Performance Example

```cpp
#include <turbokit/hash_map.h>
#include <turbokit/serialization.h>

// Custom data structure
struct UserData {
    int64_t user_id;
    std::string username;
    double score;

    template<typename Context>
    void serialize(Context& ctx) { ctx(user_id, username, score); }
};

int main() {
    // High-performance hash map
    turbokit::HashMap<int64_t, UserData> users;
    users.insert(1001, {1001, "alice", 95.5});
    users.insert(1002, {1002, "bob", 87.2});

    // Fast binary serialization
    auto buffer = turbokit::serializeToBuffer(users);

    // Deserialize back
    turbokit::HashMap<int64_t, UserData> restored_users;
    turbokit::deserializeBuffer(buffer, restored_users);

    return 0;
}
```

---

## Installation

### Using CMake FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
    turbokit
    GIT_REPOSITORY https://github.com/AlpinDale/turbokit.git
    GIT_TAG main
)
FetchContent_MakeAvailable(turbokit)

target_link_libraries(your_target PRIVATE TurboKit::TurboKit)
```

### Manual Installation

```bash
git clone https://github.com/AlpinDale/turbokit.git
cd turbokit
mkdir build && cd build
cmake .. -DTURBOKIT_BUILD_TESTS=ON -DTURBOKIT_BUILD_EXAMPLES=ON
make
make install
```

### Header-Only Usage

Simply copy the `include/` directory to your project and include the headers you need:

```cpp
#include "turbokit/clock.h"
#include "turbokit/vector.h"
// ... other headers as needed
```

---

## Core Components

### Clock - High-Precision Timing
TSC-based timing with automatic calibration for sub-microsecond precision.

**Key Features:**
- Sub-microsecond precision (~10-50ns overhead)
- Automatic calibration against system clock
- Thread-safe concurrent access
- Compatible with std::chrono

**Use Cases:**
- Performance profiling
- Real-time systems
- High-frequency trading
- Game engines

---

### Vector - Optimized Dynamic Array
Manual memory management vector with explicit capacity control.

**Key Features:**
- Manual memory management for predictable allocation
- Reserve/capacity control
- Cache-friendly layout
- STL-compatible interface

**Use Cases:**
- Performance-critical loops
- Large dataset processing
- Memory-constrained environments

---

### HashMap - High-Performance Hash Table
Custom hash table with separate chaining and optimized memory layout.

**Key Features:**
- Separate chaining collision resolution
- Custom allocator support
- Iterator support
- Open addressing optimization

**Use Cases:**
- Caching systems
- Index structures
- Configuration management

---

### Serialization - Fast Binary Serialization
Template-based serialization framework with support for complex types.

**Key Features:**
- Zero-copy for trivial types
- Template-based automatic serialization
- Support for STL containers
- Custom type serialization

**Use Cases:**
- Network protocols
- File persistence
- Inter-process communication

---

### Sync - Low-Level Synchronization
Spin mutexes and futex primitives for high-performance synchronization.

**Key Features:**
- Spin mutex implementation
- Futex-based waiting
- Thread-safe primitives
- Lock-free where possible

**Use Cases:**
- High-frequency locking
- Real-time systems
- Low-latency applications

---

### FreeList - Thread-Local Object Pooling
Memory pool implementation with thread-local storage optimization.

**Key Features:**
- Thread-local pools
- Automatic overflow to global pool
- Zero-allocation object reuse
- RAII integration

**Use Cases:**
- Object-heavy applications
- Memory allocation optimization
- Real-time systems

---

### Logging - Performance-Optimized Logging
Thread-safe logging with minimal overhead and multiple log levels.

**Key Features:**
- Multiple log levels (ERROR, INFO, VERBOSE, DEBUG)
- Thread-safe operation
- Minimal overhead design
- Printf-style formatting

**Use Cases:**
- Production debugging
- Performance monitoring
- System diagnostics

---

### Buffer - Memory Management
Reference-counted memory blocks with RAII management.

**Key Features:**
- Reference counting
- Unique and shared ownership
- RAII memory management
- Zero-copy operations

**Use Cases:**
- Large data processing
- Shared memory scenarios
- Network buffers

---

### IntrusiveList - Intrusive Data Structures
Memory-efficient doubly-linked list with intrusive nodes.

**Key Features:**
- No additional memory allocation
- O(1) insertion/deletion
- STL-compatible iterators
- Cache-friendly traversal

**Use Cases:**
- Embedded systems
- Memory-constrained environments
- Real-time data structures

---

### SimpleVector - Lightweight Vector
Simplified vector implementation with minimal overhead.

**Key Features:**
- Lightweight implementation
- Basic vector operations
- Memory-efficient
- STL-compatible interface

**Use Cases:**
- Simple use cases
- Memory-constrained environments
- Educational purposes

---

## System Requirements

- **Compiler**: GCC 7+ or Clang 6+ (C++17 support)
- **Platform**: Linux x86_64 (primary), other platforms experimental
- **Dependencies**: fmt library (automatically fetched)
- **CPU**: x86_64 with TSC support (for optimal clock performance)

---

## Support

- **Issues**: [GitHub Issues](https://github.com/AlpinDale/turbokit/issues)
- **Discussions**: [GitHub Discussions](https://github.com/AlpinDale/turbokit/discussions)
- **Documentation**: [Full API Reference](api/README.md)

---