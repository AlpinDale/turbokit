# Vector API Reference

High-performance dynamic array with manual memory management and explicit capacity control.

## Overview

The TurboKit Vector provides an optimized alternative to `std::vector` with explicit memory management, predictable performance characteristics, and cache-friendly layout. It's designed for performance-critical applications where allocation overhead and memory fragmentation matter.

## Header

```cpp
#include <turbokit/vector.h>
```

## Classes

### `turbokit::DynamicArray<ElementType, MemoryManager>`

Template class providing dynamic array functionality with manual memory management.

#### Template Parameters

- **`ElementType`**: Type of elements stored in the array
- **`MemoryManager`**: Allocator type (default: `std::allocator<ElementType>`)

#### Type Definitions

```cpp
using value_type = ElementType;
```

#### Constructors

##### Default Constructor

```cpp
DynamicArray();
```

Creates an empty array with no allocated memory.

**Performance:**
- **Time Complexity:** O(1)
- **Memory Usage:** Zero allocation

##### Copy Constructor

```cpp
DynamicArray(const DynamicArray& other);
```

Creates a deep copy of another array.

**Performance:**
- **Time Complexity:** O(n) where n is `other.size()`
- **Memory Usage:** Allocates capacity equal to `other.size()`

##### Move Constructor

```cpp
DynamicArray(DynamicArray&& other) noexcept;
```

Transfers ownership from another array.

**Performance:**
- **Time Complexity:** O(1)
- **Memory Usage:** No additional allocation

##### Size Constructor

```cpp
explicit DynamicArray(size_t initial_size);
```

Creates an array with the specified size, elements are default-constructed.

**Parameters:**
- `initial_size`: Number of elements to create

**Performance:**
- **Time Complexity:** O(n) where n is `initial_size`
- **Memory Usage:** Allocates exactly `initial_size` elements

#### Destructor

```cpp
~DynamicArray();
```

Destroys all elements and deallocates memory.

**Performance:**
- **Time Complexity:** O(n) where n is `size()`

#### Assignment Operators

##### Copy Assignment

```cpp
DynamicArray& operator=(const DynamicArray& other);
```

Replaces contents with a copy of another array.

**Performance:**
- **Time Complexity:** O(n + m) where n is current size, m is `other.size()`

##### Move Assignment

```cpp
DynamicArray& operator=(DynamicArray&& other) noexcept;
```

Replaces contents by transferring ownership.

**Performance:**
- **Time Complexity:** O(1)

#### Element Access

##### `get_at()`

```cpp
ElementType& get_at(size_t position);
const ElementType& get_at(size_t position) const;
```

Returns a reference to the element at the specified position with bounds checking.

**Parameters:**
- `position`: Index of element to access

**Returns:**
- Reference to the element at the specified position

**Throws:**
- `std::out_of_range`: If position >= size()

**Performance:**
- **Time Complexity:** O(1)

##### `operator[]`

```cpp
ElementType& operator[](size_t position);
const ElementType& operator[](size_t position) const;
```

Returns a reference to the element at the specified position without bounds checking.

**Parameters:**
- `position`: Index of element to access

**Returns:**
- Reference to the element at the specified position

**Performance:**
- **Time Complexity:** O(1)
- **Note:** No bounds checking in release builds for maximum performance

#### Data Access

##### `get_data()`

```cpp
ElementType* get_data();
const ElementType* get_data() const;
```

Returns a pointer to the underlying data array.

**Returns:**
- Pointer to the first element, or nullptr if empty

**Performance:**
- **Time Complexity:** O(1)

##### `get_begin()` / `get_end()`

```cpp
ElementType* get_begin();
ElementType* get_end();
const ElementType* get_begin() const;
const ElementType* get_end() const;
```

Returns pointers to the beginning and end of the data.

**Returns:**
- `get_begin()`: Pointer to first element
- `get_end()`: Pointer one past the last element

**Performance:**
- **Time Complexity:** O(1)

#### Iterator Support

##### `begin()` / `end()`

```cpp
ElementType* begin();
ElementType* end();
const ElementType* begin() const;
const ElementType* end() const;
```

Returns iterators for range-based for loops and STL algorithms.

**Returns:**
- `begin()`: Iterator to first element
- `end()`: Iterator one past the last element

**Performance:**
- **Time Complexity:** O(1)

**Example:**
```cpp
turbokit::Vector<int> vec = {1, 2, 3, 4, 5};

// Range-based for loop
for (auto& value : vec) {
    value *= 2;
}

// STL algorithms
std::sort(vec.begin(), vec.end());
```

#### Capacity Management

##### `size()`

```cpp
size_t size() const;
```

Returns the number of elements in the array.

**Returns:**
- Number of elements currently stored

**Performance:**
- **Time Complexity:** O(1)

##### `get_capacity()`

```cpp
size_t get_capacity() const;
```

Returns the current capacity of the array.

**Returns:**
- Maximum number of elements that can be stored without reallocation

**Performance:**
- **Time Complexity:** O(1)

##### `reserve()`

```cpp
void reserve(size_t new_capacity);
```

Reserves memory for at least the specified number of elements.

**Parameters:**
- `new_capacity`: Minimum capacity to reserve

**Performance:**
- **Time Complexity:** O(n) if reallocation occurs, O(1) otherwise
- **Memory Usage:** Allocates memory for `new_capacity` elements

**Example:**
```cpp
turbokit::Vector<int> vec;
vec.reserve(1000000); // Pre-allocate for 1M elements
for (int i = 0; i < 1000000; ++i) {
    vec.append(i); // No reallocations
}
```

##### `resize()`

```cpp
void resize(size_t new_size);
```

Changes the size of the array, constructing or destroying elements as needed.

**Parameters:**
- `new_size`: New size of the array

**Performance:**
- **Time Complexity:** O(|new_size - size()|)
- **Memory Usage:** May trigger reallocation if `new_size > capacity()`

#### Element Modification

##### `append()`

```cpp
void append(const ElementType& value);
void append(ElementType&& value);
```

Adds an element to the end of the array.

**Parameters:**
- `value`: Element to add

**Performance:**
- **Time Complexity:** O(1) amortized, O(n) worst case (reallocation)
- **Memory Usage:** May trigger reallocation with exponential growth

**Example:**
```cpp
turbokit::Vector<std::string> names;
names.append("Alice");              // Copy
names.append(std::string("Bob"));   // Move
```

##### `pop_back()`

```cpp
void pop_back();
```

Removes the last element from the array.

**Preconditions:**
- Array must not be empty

**Performance:**
- **Time Complexity:** O(1)

##### `clear()`

```cpp
void clear();
```

Removes all elements from the array, but retains allocated memory.

**Performance:**
- **Time Complexity:** O(n) where n is `size()`
- **Memory Usage:** No deallocation occurs

##### `reset()`

```cpp
void reset();
```

Removes all elements and deallocates memory.

**Performance:**
- **Time Complexity:** O(n) where n is `size()`
- **Memory Usage:** All memory is deallocated

#### Utility Functions

##### `is_empty()`

```cpp
bool is_empty() const;
```

Returns whether the array is empty.

**Returns:**
- `true` if size() == 0, `false` otherwise

**Performance:**
- **Time Complexity:** O(1)

### `turbokit::Vector<ElementType>`

Type alias for the most commonly used DynamicArray configuration.

```cpp
template<typename ElementType>
using Vector = DynamicArray<ElementType, std::allocator<ElementType>>;
```

## Performance Characteristics

### Time Complexity

| Operation    | Best Case | Average Case | Worst Case | Notes                            |
| ------------ | --------- | ------------ | ---------- | -------------------------------- |
| `append()`   | O(1)      | O(1)         | O(n)       | Worst case during reallocation   |
| `operator[]` | O(1)      | O(1)         | O(1)       | No bounds checking               |
| `get_at()`   | O(1)      | O(1)         | O(1)       | With bounds checking             |
| `pop_back()` | O(1)      | O(1)         | O(1)       |                                  |
| `resize()`   | O(1)      | O(k)         | O(n)       | k =                              |new_size - size()|
| `reserve()`  | O(1)      | O(1)         | O(n)       | Worst case during reallocation   |
| `clear()`    | O(1)      | O(n)         | O(n)       | Depends on destructor complexity |
|              |           |              |            |                                  |

### Memory Usage

- **Overhead per container:** ~32-48 bytes (pointers + counters)
- **Overhead per element:** 0 bytes
- **Growth factor:** 1.5x (configurable)
- **Alignment:** Respects `alignof(ElementType)`

### Cache Performance

- **Sequential access:** Optimal cache locality
- **Random access:** Good cache performance for small arrays
- **Memory layout:** Contiguous allocation
- **Prefetching:** Benefits from hardware prefetchers

## Use Cases

### 1. High-Performance Computing

```cpp
#include <turbokit/vector.h>
#include <turbokit/clock.h>

void process_large_dataset() {
    turbokit::Vector<double> data;
    data.reserve(10000000); // Pre-allocate for 10M elements

    auto start = turbokit::clock.get_current_time();

    // Load data efficiently
    for (int i = 0; i < 10000000; ++i) {
        data.append(compute_value(i));
    }

    // Process with optimal cache access
    double sum = 0.0;
    for (const auto& value : data) {
        sum += value;
    }

    auto end = turbokit::clock.get_current_time();
    turbokit::log.info("Processed 10M elements in %lld ns", end - start);
}
```

### 2. Real-Time Systems

```cpp
#include <turbokit/vector.h>

class RealTimeProcessor {
private:
    turbokit::Vector<Message> message_buffer;

public:
    RealTimeProcessor() {
        // Pre-allocate to avoid runtime allocations
        message_buffer.reserve(10000);
    }

    void process_messages(const Message* msgs, size_t count) {
        // Clear without deallocation
        message_buffer.clear();

        // Copy messages efficiently
        for (size_t i = 0; i < count; ++i) {
            message_buffer.append(msgs[i]);
        }

        // Process in-place
        for (auto& msg : message_buffer) {
            process_single_message(msg);
        }
    }
};
```

### 3. Memory-Constrained Environments

```cpp
#include <turbokit/vector.h>

class MemoryEfficientContainer {
private:
    turbokit::Vector<uint32_t> indices;

public:
    void optimize_memory() {
        // Shrink to exact size needed
        turbokit::Vector<uint32_t> temp;
        temp.reserve(indices.size());

        for (const auto& index : indices) {
            temp.append(index);
        }

        // Replace with exactly-sized container
        indices = std::move(temp);
    }

    size_t memory_usage() const {
        return indices.get_capacity() * sizeof(uint32_t) + sizeof(indices);
    }
};
```

### 4. Custom Allocators

```cpp
#include <turbokit/vector.h>
#include <memory_resource>

void use_custom_allocator() {
    // Pool allocator for better performance
    std::pmr::monotonic_buffer_resource pool(65536); // 64KB pool
    std::pmr::polymorphic_allocator<int> alloc(&pool);

    turbokit::DynamicArray<int, std::pmr::polymorphic_allocator<int>> vec(alloc);
    vec.reserve(1000);

    for (int i = 0; i < 1000; ++i) {
        vec.append(i);
    }

    // All allocations came from the pool
}
```

## Best Practices

### 1. Pre-allocate When Possible

```cpp
// ❌ Bad: Multiple reallocations
turbokit::Vector<int> vec;
for (int i = 0; i < 1000000; ++i) {
    vec.append(i); // May cause many reallocations
}

// ✅ Good: Single allocation
turbokit::Vector<int> vec;
vec.reserve(1000000); // Allocate once
for (int i = 0; i < 1000000; ++i) {
    vec.append(i); // No reallocations
}
```

### 2. Use Move Semantics

```cpp
// ❌ Bad: Unnecessary copies
turbokit::Vector<std::string> names;
std::string name = "Alice";
names.append(name); // Copy

// ✅ Good: Move when possible
turbokit::Vector<std::string> names;
names.append(std::string("Alice")); // Temporary, moved
names.append(std::move(name));      // Explicit move
```

### 3. Batch Operations

```cpp
// ❌ Bad: Element-by-element processing
turbokit::Vector<int> src, dst;
for (const auto& value : src) {
    if (should_copy(value)) {
        dst.append(value);
    }
}

// ✅ Good: Reserve and batch
turbokit::Vector<int> src, dst;
dst.reserve(src.size()); // Worst-case allocation
for (const auto& value : src) {
    if (should_copy(value)) {
        dst.append(value);
    }
}
```

### 4. Memory Management

```cpp
// Control memory usage explicitly
turbokit::Vector<LargeObject> objects;

// For temporary usage
objects.reserve(expected_size);
// ... use objects ...
objects.reset(); // Free memory immediately

// For reusable containers
objects.clear(); // Keep memory allocated for reuse
```

## Migration from std::vector

### Basic Replacement

```cpp
// Before
std::vector<int> vec;
vec.push_back(42);
vec.reserve(1000);
auto size = vec.size();

// After
turbokit::Vector<int> vec;
vec.append(42);           // push_back -> append
vec.reserve(1000);        // Same
auto size = vec.size();   // Same
```

### Performance Considerations

```cpp
// std::vector - implicit memory management
std::vector<int> vec;
for (int i = 0; i < 1000000; ++i) {
    vec.push_back(i); // Growth strategy hidden
}

// TurboKit Vector - explicit control
turbokit::Vector<int> vec;
vec.reserve(1000000);     // Explicit pre-allocation
for (int i = 0; i < 1000000; ++i) {
    vec.append(i);        // Guaranteed O(1)
}
```

## Thread Safety

The Vector class is **not thread-safe**. External synchronization is required for concurrent access:

```cpp
#include <turbokit/sync.h>

turbokit::Vector<int> shared_vector;
turbokit::SpinMutex vector_mutex;

void thread_safe_append(int value) {
    std::lock_guard<turbokit::SpinMutex> lock(vector_mutex);
    shared_vector.append(value);
}
```

## Platform Considerations

### Memory Alignment

Vector respects the alignment requirements of the element type:

```cpp
// Automatically aligned for SIMD operations
turbokit::Vector<__m256> simd_data;
static_assert(alignof(__m256) == 32);
```

### Large Objects

For large objects, consider using a vector of pointers or unique_ptr:

```cpp
// Instead of
turbokit::Vector<VeryLargeObject> objects; // May cause excessive copying

// Consider
turbokit::Vector<std::unique_ptr<VeryLargeObject>> objects;
```

---

**See Also:**
- [SimpleVector API](simple_vector.md) - Lightweight alternative
- [Performance Guide](../performance.md) - Optimization techniques
- [Examples](../examples/containers.md) - Container usage examples