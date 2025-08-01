# Clock API Reference

High-precision timing with TSC-based implementation and automatic calibration.

## Overview

The TurboKit Clock provides sub-microsecond precision timing with minimal overhead. It uses the x86 Time Stamp Counter (TSC) with automatic calibration against the system clock to provide accurate time measurements.

## Header

```cpp
#include <turbokit/clock.h>
```

## Classes

### `turbokit::Clock`

Primary clock implementation with TSC-based timing.

#### Member Functions

##### `get_current_time()`

```cpp
int64_t get_current_time();
```

Returns the current time in nanoseconds since an arbitrary epoch.

**Returns:**
- `int64_t`: Current time in nanoseconds

**Performance:**
- **Time Complexity:** O(1)
- **Typical Overhead:** 10-50 nanoseconds
- **Thread Safety:** Thread-safe

**Example:**
```cpp
auto start = turbokit::clock.get_current_time();
// ... work ...
auto end = turbokit::clock.get_current_time();
auto duration_ns = end - start;
```

**Notes:**
- Uses TSC (Time Stamp Counter) for maximum performance
- Automatically calibrates against system clock
- Returns monotonically increasing values
- Sub-microsecond precision on modern x86_64 systems

---

### `turbokit::HighPerformanceClock`

STL-compatible clock interface following `std::chrono` conventions.

#### Type Definitions

```cpp
using duration = std::chrono::nanoseconds;
using rep = duration::rep;
using period = duration::period;
using time_point = std::chrono::time_point<HighPerformanceClock, duration>;
static constexpr bool is_steady = true;
```

#### Static Member Functions

##### `now()`

```cpp
static time_point now() noexcept;
```

Returns the current time as a `time_point`.

**Returns:**
- `time_point`: Current time compatible with `std::chrono`

**Performance:**
- **Time Complexity:** O(1)
- **Typical Overhead:** 10-50 nanoseconds
- **Thread Safety:** Thread-safe

**Example:**
```cpp
auto start = turbokit::HighPerformanceClock::now();
std::this_thread::sleep_for(std::chrono::microseconds(100));
auto end = turbokit::HighPerformanceClock::now();

auto duration = end - start;
auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
std::cout << "Duration: " << duration_ns.count() << " ns\n";
```

## Global Objects

### `turbokit::clock`

```cpp
inline Clock& clock = time_manager;
```

Global clock instance ready for immediate use.

**Example:**
```cpp
auto timestamp = turbokit::clock.get_current_time();
```

### `turbokit::time_manager`

```cpp
inline Clock time_manager;
```

The underlying clock implementation instance.

## Type Aliases

```cpp
using Clock = Clock;                    // Primary clock type
using FastClock = HighPerformanceClock; // STL-compatible alias
```

## Implementation Details

### TSC Calibration

The clock automatically calibrates the TSC frequency against the system clock:

1. **Initial Calibration:** Performed on first use
2. **Periodic Recalibration:** Every 1 second to handle frequency changes
3. **Threshold Management:** Automatic switching between TSC and system clock

### Thread Safety

The clock implementation provides thread-safe access through several mechanisms:

- **Atomic Operations:** Reference counting and calibration data
- **Memory Ordering:** Careful use of memory barriers
- **Lock-Free Design:** Most operations are lock-free
- **Fallback Synchronization:** Minimal locking only during calibration

### Error Handling

The clock provides graceful degradation:

- **TSC Unavailable:** Falls back to `std::chrono::steady_clock`
- **Frequency Changes:** Automatic recalibration
- **Overflow Protection:** Handles wraparound conditions
- **Invalid Measurements:** Filters outliers during calibration

## Performance Characteristics

### Precision and Accuracy

| Metric | Value | Notes |
|--------|-------|-------|
| **Precision** | ~1-10 ns | Limited by TSC resolution |
| **Accuracy** | ±0.1% | Calibrated against system clock |
| **Stability** | ±10 ppm | Over temperature/voltage variations |
| **Overhead** | 10-50 ns | Per `get_current_time()` call |

### Calibration Overhead

| Operation | Frequency | Overhead |
|-----------|-----------|----------|
| **Normal Call** | Every call | 10-50 ns |
| **Calibration Check** | Every ~1000 calls | +5-10 ns |
| **Full Calibration** | Every 1 second | +1-10 μs |

### Scalability

- **Single Thread:** Up to 100M calls/second
- **Multiple Threads:** Scales linearly with core count
- **Cache Effects:** Minimal L1 cache footprint
- **Memory Bandwidth:** ~1-2 bytes per call

## Use Cases

### 1. Performance Profiling

```cpp
#include <turbokit/clock.h>

void profile_function() {
    auto start = turbokit::clock.get_current_time();

    // Function to profile
    expensive_computation();

    auto end = turbokit::clock.get_current_time();
    auto duration_ns = end - start;

    if (duration_ns > 1000000) { // > 1ms
        turbokit::log.warning("Slow operation: %lld ns", duration_ns);
    }
}
```

### 2. High-Frequency Measurements

```cpp
#include <turbokit/clock.h>
#include <turbokit/vector.h>

void measure_latencies() {
    turbokit::Vector<int64_t> latencies;
    latencies.reserve(1000000);

    for (int i = 0; i < 1000000; ++i) {
        auto start = turbokit::clock.get_current_time();

        // Critical path measurement
        process_single_item();

        auto end = turbokit::clock.get_current_time();
        latencies.append(end - start);
    }

    // Analyze latency distribution
    std::sort(latencies.begin(), latencies.end());
    auto p99 = latencies[latencies.size() * 99 / 100];
    turbokit::log.info("P99 latency: %lld ns", p99);
}
```

### 3. Real-Time Systems

```cpp
#include <turbokit/clock.h>

class RealTimeScheduler {
private:
    int64_t deadline_ns;

public:
    void set_deadline(int64_t timeout_ns) {
        deadline_ns = turbokit::clock.get_current_time() + timeout_ns;
    }

    bool has_time_remaining() const {
        return turbokit::clock.get_current_time() < deadline_ns;
    }

    int64_t remaining_time() const {
        auto now = turbokit::clock.get_current_time();
        return std::max(0LL, deadline_ns - now);
    }
};
```

### 4. STL Integration

```cpp
#include <turbokit/clock.h>
#include <chrono>

void stl_compatibility_example() {
    // Use with STL algorithms
    auto start = turbokit::HighPerformanceClock::now();

    // STL duration arithmetic
    auto timeout = std::chrono::milliseconds(100);
    auto deadline = start + timeout;

    while (turbokit::HighPerformanceClock::now() < deadline) {
        // Work until deadline
        process_batch();
    }

    // Duration calculation
    auto end = turbokit::HighPerformanceClock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Elapsed: " << elapsed.count() << " μs\n";
}
```

## Best Practices

### 1. Minimize Clock Calls

```cpp
// ❌ Bad: Clock call in tight loop
for (int i = 0; i < 1000000; ++i) {
    auto start = turbokit::clock.get_current_time();
    process_item(i);
    auto end = turbokit::clock.get_current_time();
    record_latency(end - start);
}

// ✅ Good: Batch measurements
auto start = turbokit::clock.get_current_time();
for (int i = 0; i < 1000000; ++i) {
    process_item(i);
}
auto end = turbokit::clock.get_current_time();
auto avg_latency = (end - start) / 1000000;
```

### 2. Handle Calibration Delays

```cpp
// ❌ Bad: Assumes constant overhead
auto start = turbokit::clock.get_current_time();
auto end = turbokit::clock.get_current_time();
// This could include calibration overhead!

// ✅ Good: Warm up the clock
for (int i = 0; i < 10; ++i) {
    turbokit::clock.get_current_time(); // Warm up
}
auto start = turbokit::clock.get_current_time();
// ... measurement ...
auto end = turbokit::clock.get_current_time();
```

### 3. Use Appropriate Time Units

```cpp
// ✅ Good: Clear time units
auto duration_ns = end - start;
auto duration_us = duration_ns / 1000;
auto duration_ms = duration_ns / 1000000;

// Or use chrono for clarity
auto duration = std::chrono::nanoseconds(duration_ns);
auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
```

## Platform Considerations

### Linux x86_64 (Primary Platform)

- **TSC Support:** Full TSC implementation with RDTSC instruction
- **Calibration:** Uses CLOCK_MONOTONIC for calibration
- **Performance:** Optimal performance with all features

### Other Platforms

- **macOS x86_64:** TSC support with reduced calibration accuracy
- **Windows x86_64:** Basic TSC support, compatibility issues possible
- **ARM64:** Falls back to system clock, reduced performance
- **32-bit Systems:** Not supported due to 64-bit timestamp requirements

### CPU Requirements

- **TSC (Time Stamp Counter):** Required for optimal performance
- **Constant TSC:** Recommended for accuracy across frequency changes
- **Invariant TSC:** Ideal for multi-core systems

## Troubleshooting

### Common Issues

1. **High Overhead:** Check for frequent calibration due to TSC instability
2. **Negative Durations:** Ensure proper time ordering in multi-threaded code
3. **Poor Precision:** Verify TSC support on target hardware
4. **Calibration Warnings:** May indicate hardware issues or virtualization

### Debug Build Features

```cpp
#ifdef TURBOKIT_DEBUG
    // Additional validation and logging
    auto time1 = turbokit::clock.get_current_time();
    auto time2 = turbokit::clock.get_current_time();
    assert(time2 >= time1); // Monotonicity check
#endif
```

### Performance Monitoring

```cpp
// Monitor calibration frequency
extern std::atomic<size_t> calibration_count;
size_t calibrations_before = calibration_count.load();

// ... your code ...

size_t calibrations_after = calibration_count.load();
if (calibrations_after > calibrations_before) {
    turbokit::log.warning("Clock recalibrated %zu times",
                         calibrations_after - calibrations_before);
}
```

---

**See Also:**
- [Performance Guide](../performance.md) - Optimization techniques
- [Examples](../examples/timing.md) - Complete timing examples
- [Migration Guide](../migration.md) - From std::chrono to TurboKit