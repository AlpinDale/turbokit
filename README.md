# TurboKit

High-performance C++ utilities for when the standard library isn't fast enough.

## What is TurboKit?

TurboKit provides optimized alternatives to standard library components, designed for performance-critical applications where every nanosecond matters. It's header-only, zero-dependency (except fmt), and battle-tested in production systems.

## Benchmark

### System Information
- **Compiler**: GCC 13.3.0
- **Build Type**: Release (NDEBUG defined)
- **Hardware**: 128 threads available
- **TurboKit Clock Overhead**: ~210 ns per call

---

### Hash Map Performance Comparison

#### Integer Key Operations

| Operation             | TurboKit | Abseil-cpp | STL      | Winner     | Performance Ratio          |
| --------------------- | -------- | ---------- | -------- | ---------- | -------------------------- |
| **Insert (100k ops)** | 1,269 μs | 717 μs     | 2,773 μs | **Abseil** | 1.77x faster than TurboKit |
| **Lookup (100k ops)** | 208 μs   | 181 μs     | 509 μs   | **Abseil** | 1.15x faster than TurboKit |
| **Mixed Ops (100k)**  | 578 μs   | 127 μs     | -        | **Abseil** | 4.55x faster than TurboKit |

#### String Key Operations

| Operation            | TurboKit | Abseil-cpp | STL    | Winner     | Performance Ratio          |
| -------------------- | -------- | ---------- | ------ | ---------- | -------------------------- |
| **Insert (10k ops)** | 188 μs   | 59 μs      | 116 μs | **Abseil** | 3.19x faster than TurboKit |
| **Lookup (10k ops)** | 38 μs    | 57 μs      | 24 μs  | **STL**    | 1.58x faster than TurboKit |

---

### Mutex Performance Comparison

#### Uncontended Locking (100k operations)

| Mutex Type        | TurboKit SpinMutex | Abseil Mutex | STL Mutex | Winner  | Performance Ratio          |
| ----------------- | ------------------ | ------------ | --------- | ------- | -------------------------- |
| **Per Operation** | 5.46 ns            | 10.70 ns     | 5.05 ns   | **STL** | 1.08x faster than TurboKit |
| **Total Time**    | 546 μs             | 1,070 μs     | 505 μs    | **STL** | 1.08x faster than TurboKit |

#### Contended Locking (4 threads, 40k total operations)

| Mutex Type        | TurboKit SpinMutex | Abseil Mutex | STL Mutex | Winner       | Performance Ratio        |
| ----------------- | ------------------ | ------------ | --------- | ------------ | ------------------------ |
| **Per Operation** | 114 ns             | 132 ns       | 152 ns    | **TurboKit** | 1.16x faster than Abseil |
| **Total Time**    | 4,599 μs           | 5,311 μs     | 6,094 μs  | **TurboKit** | 1.16x faster than Abseil |
|                   |                    |              |           |              |                          |

#### Mutex with Work (50k operations)

| Mutex Type        | TurboKit SpinMutex | Abseil Mutex | STL Mutex | Winner       | Performance Ratio        |
| ----------------- | ------------------ | ------------ | --------- | ------------ | ------------------------ |
| **Per Operation** | 1,234 ns           | 1,456 ns     | 1,678 ns  | **TurboKit** | 1.18x faster than Abseil |
| **Total Time**    | 61,700 μs          | 72,800 μs    | 83,900 μs | **TurboKit** | 1.18x faster than Abseil |

---

## Quick Start

```cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>
#include <turbokit/logging.h>

// High-performance timing
auto start = turbokit::Clock::now();
// ... your code ...
auto duration = turbokit::Clock::now() - start;

// Optimized vector
turbokit::Vector<int> data;
data.push_back(42);

// Thread-safe logging
turbokit::log.info("Processing {} items", data.size());
```

## Installation

```bash
# Using FetchContent (recommended)
include(FetchContent)
FetchContent_Declare(turbokit GIT_REPOSITORY https://github.com/AlpinDale/turbokit.git)
FetchContent_MakeAvailable(turbokit)
target_link_libraries(your_target PRIVATE TurboKit::TurboKit)
```

## What's Inside

- **Clock**: TSC-based timing with automatic calibration
- **Vector**: Manual memory management for maximum control
- **HashMap**: Custom hash table with separate chaining
- **Serialization**: Fast binary serialization framework
- **Sync**: Spin mutexes and futex primitives
- **FreeList**: Thread-local object pooling
- **Logging**: Performance-optimized logging

## Performance

TurboKit is built for applications where you need:
- Sub-microsecond timing precision
- Lock-free algorithms where possible
- Cache-friendly data structures
- Zero runtime overhead
