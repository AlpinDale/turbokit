#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "buffer.h"
#include "clock.h"
#include "hash_map.h"
#include "logging.h"
#include "serialization.h"
#include "sync.h"
#include "vector.h"

// NOTE: don't use 'using namespace' to avoid conflicts with std::clock and
// std::log

// Test struct for serialization
struct TestData {
  int id;
  std::string name;
  double value;

  template <typename X> void serialize(X &x) { x(id, name, value); }
};

void demonstrateClock() {
  std::cout << "\n=== TurboKit Clock Demo ===\n";

  // High-precision timing
  auto start = turbokit::clock.get_current_time();
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  auto end = turbokit::clock.get_current_time();

  auto duration_ns = end - start;
  auto duration_ms = duration_ns / 1'000'000;

  std::cout << "Measured sleep: " << duration_ms << " ms (" << duration_ns
            << " ns)\n";

  // Clock overhead benchmark
  const int iterations = 10000;
  auto bench_start = turbokit::clock.get_current_time();
  for (int i = 0; i < iterations; ++i) {
    volatile auto t = turbokit::clock.get_current_time();
    (void)t;
  }
  auto bench_end = turbokit::clock.get_current_time();

  auto overhead_ns = (bench_end - bench_start) / iterations;
  std::cout << "Clock overhead: ~" << overhead_ns << " ns per call\n";
}

void demonstrateVector() {
  std::cout << "\n=== TurboKit Vector Demo ===\n";

  turbokit::Vector<int> vec;

  // Add some data
  for (int i = 0; i < 10; ++i) {
    vec.append(i * i);
  }

  std::cout << "Vector size: " << vec.size() << ", capacity: " << vec.get_capacity()
            << "\n";
  std::cout << "Elements: ";
  for (size_t i = 0; i < vec.size(); ++i) {
    std::cout << vec[i] << " ";
  }
  std::cout << "\n";

  // Performance test
  const int perf_size = 100000;
  turbokit::Vector<int> perf_vec;

  auto start = turbokit::clock.get_current_time();
  for (int i = 0; i < perf_size; ++i) {
    perf_vec.append(i);
  }
  auto end = turbokit::clock.get_current_time();

  auto duration_us = (end - start) / 1000;
  std::cout << "Performance: Added " << perf_size << " elements in "
            << duration_us << " μs\n";
}

void demonstrateHashMap() {
  std::cout << "\n=== TurboKit HashMap Demo ===\n";

  turbokit::HashMap<std::string, int> inventory;

  // Add some items
  inventory.insert("apples", 50);
  inventory.insert("bananas", 30);
  inventory.insert("oranges", 25);

  std::cout << "Inventory size: " << inventory.size() << "\n";

  // Look up an item
  auto it = inventory.find("bananas");
  if (it != inventory.end()) {
    std::cout << "Found: " << it->first << " = " << it->second << " units\n";
  }

  // Performance test
  const int perf_size = 50000;
  turbokit::HashMap<int, int> perf_map;

  auto start = turbokit::clock.get_current_time();
  for (int i = 0; i < perf_size; ++i) {
    perf_map.insert(i, i * 2);
  }
  auto end = turbokit::clock.get_current_time();

  auto duration_us = (end - start) / 1000;
  std::cout << "Performance: Inserted " << perf_size << " pairs in "
            << duration_us << " μs\n";
}

void demonstrateLogging() {
  std::cout << "\n=== TurboKit Logging Demo ===\n";

  turbokit::log.info("Application started");
  turbokit::log.error("This is a sample error message");
  turbokit::log.verbose("Verbose information");

  // Formatted logging
  int items = 42;
  double price = 19.99;
  turbokit::log.info("Processing %d items at $%.2f each", items, price);

  std::cout << "Check above for log messages with timestamps\n";
}

void demonstrateSerialization() {
  std::cout << "\n=== TurboKit Serialization Demo ===\n";

  // Create test data
  TestData original{123, "example_data", 3.14159};

  std::cout << "Original: id=" << original.id << ", name=" << original.name
            << ", value=" << original.value << "\n";

  // Serialize
  auto buffer = turbokit::serializeToBuffer(original);
  std::cout << "Serialized to " << buffer->get_size() << " bytes\n";

  // Deserialize
  TestData restored;
  turbokit::deserializeBuffer(buffer, restored);

  std::cout << "Restored: id=" << restored.id << ", name=" << restored.name
            << ", value=" << restored.value << "\n";

  bool success = (original.id == restored.id) &&
                 (original.name == restored.name) &&
                 (original.value == restored.value);

  std::cout << "Serialization test: " << (success ? "PASSED" : "FAILED")
            << "\n";
}

void demonstrateBuffer() {
  std::cout << "\n=== TurboKit Buffer Demo ===\n";

  const size_t size = 1024;
  auto buffer = turbokit::Buffer::create(size);

  std::cout << "Allocated buffer: " << buffer->get_size() << " bytes\n";

  // Write pattern
  auto *data = buffer->get_data();
  for (size_t i = 0; i < size; ++i) {
    data[i] = std::byte(i % 256);
  }

  // Verify pattern
  bool pattern_ok = true;
  for (size_t i = 0; i < size && pattern_ok; ++i) {
    if (data[i] != std::byte(i % 256)) {
      pattern_ok = false;
    }
  }

  std::cout << "Buffer pattern test: " << (pattern_ok ? "PASSED" : "FAILED")
            << "\n";

  // Use RAII handle
  {
    turbokit::BufferHandle handle(buffer);
    std::cout << "Buffer managed by handle: " << handle->get_size() << " bytes\n";
    buffer = nullptr; // Transfer ownership
  }
  std::cout << "Buffer automatically cleaned up\n";
}

void demonstrateSync() {
  std::cout << "\n=== TurboKit Sync Demo ===\n";

  turbokit::SpinMutex mutex;

  // Test basic locking
  {
    std::lock_guard<turbokit::SpinMutex> lock(mutex);
    std::cout << "Acquired spin lock\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  std::cout << "Released spin lock\n";

  // Performance test
  const int lock_count = 50000;
  auto start = turbokit::clock.get_current_time();

  for (int i = 0; i < lock_count; ++i) {
    std::lock_guard<turbokit::SpinMutex> lock(mutex);
    // Critical section
  }

  auto end = turbokit::clock.get_current_time();
  auto duration_us = (end - start) / 1000;

  std::cout << "Spin lock performance: " << lock_count << " cycles in "
            << duration_us << " μs\n";
}

int main() {
  std::cout << "TurboKit Example - High-Performance C++ Utilities\n";
  std::cout << "=================================================\n";

  try {
    demonstrateClock();
    demonstrateVector();
    demonstrateHashMap();
    demonstrateLogging();
    demonstrateSerialization();
    demonstrateBuffer();
    demonstrateSync();

  } catch (const std::exception &e) {
    std::cerr << "\nError: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}