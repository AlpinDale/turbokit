# HashMap API Reference

High-performance hash table implementation with separate chaining and optimized memory layout.

## Overview

The TurboKit HashMap provides a high-performance alternative to `std::unordered_map` with predictable performance characteristics, cache-friendly design, and minimal overhead. It uses separate chaining for collision resolution with optimized memory layout for better cache performance.

## Header

```cpp
#include <turbokit/hash_map.h>
```

## Classes

### `turbokit::HashMap<KeyType, ValueType, HashFunction, EqualityFunction, MemoryAllocator>`

Template class providing hash table functionality with separate chaining collision resolution.

#### Template Parameters

- **`KeyType`**: Type of the keys
- **`ValueType`**: Type of the values
- **`HashFunction`**: Hash function object (default: `std::hash<KeyType>`)
- **`EqualityFunction`**: Equality comparison function (default: `std::equal_to<KeyType>`)
- **`MemoryAllocator`**: Allocator type (default: `std::allocator<void>`)

#### Internal Types

```cpp
struct MainEntry {
    KeyType key;
    ValueType value;
    size_t chain_length;
};

struct CollisionEntry {
    KeyType key;
    ValueType value;
    size_t main_index;
};
```

#### Iterator

##### `HashMap::iterator`

Bidirectional iterator for traversing the hash map.

```cpp
struct iterator {
    using difference_type = std::ptrdiff_t;
    using value_type = ValueType;
    using pointer = ValueType*;
    using reference = ValueType&;
    using iterator_category = std::bidirectional_iterator_tag;

    // Iterator operations
    std::pair<KeyType&, ValueType&>& operator*() const noexcept;
    std::pair<KeyType&, ValueType&>* operator->() const noexcept;
    iterator& operator++() noexcept;
    iterator operator++(int) noexcept;
    bool operator==(iterator other) const noexcept;
    bool operator!=(iterator other) const noexcept;
};
```

#### Constructors

##### Default Constructor

```cpp
HashMap();
```

Creates an empty hash map with default initial capacity.

**Performance:**
- **Time Complexity:** O(1)
- **Memory Usage:** Minimal allocation for empty state

##### Copy Constructor

```cpp
HashMap(const HashMap& other);
```

Creates a deep copy of another hash map.

**Performance:**
- **Time Complexity:** O(n) where n is `other.size()`
- **Memory Usage:** Allocates capacity equal to `other.bucket_count`

##### Move Constructor

```cpp
HashMap(HashMap&& other) noexcept;
```

Transfers ownership from another hash map.

**Performance:**
- **Time Complexity:** O(1)
- **Memory Usage:** No additional allocation

#### Destructor

```cpp
~HashMap();
```

Destroys all elements and deallocates memory.

**Performance:**
- **Time Complexity:** O(n) where n is `size()`

#### Assignment Operators

##### Copy Assignment

```cpp
HashMap& operator=(const HashMap& other);
```

Replaces contents with a copy of another hash map.

**Performance:**
- **Time Complexity:** O(n + m) where n is current size, m is `other.size()`

##### Move Assignment

```cpp
HashMap& operator=(HashMap&& other) noexcept;
```

Replaces contents by transferring ownership.

**Performance:**
- **Time Complexity:** O(1)

#### Element Access and Modification

##### `insert()`

```cpp
std::pair<iterator, bool> insert(const KeyType& key, const ValueType& value);
std::pair<iterator, bool> insert(KeyType&& key, ValueType&& value);
```

Inserts a key-value pair into the hash map.

**Parameters:**
- `key`: Key to insert
- `value`: Value to associate with the key

**Returns:**
- `std::pair<iterator, bool>`: Iterator to the element and boolean indicating if insertion occurred

**Performance:**
- **Time Complexity:** O(1) average case, O(n) worst case
- **Memory Usage:** May trigger rehashing for large load factors

**Example:**
```cpp
turbokit::HashMap<std::string, int> ages;
auto [it, inserted] = ages.insert("Alice", 25);
if (inserted) {
    turbokit::log.info("Inserted new entry");
} else {
    turbokit::log.info("Key already exists");
}
```

##### `find()`

```cpp
iterator find(const KeyType& key);
const_iterator find(const KeyType& key) const;
```

Finds an element with the specified key.

**Parameters:**
- `key`: Key to search for

**Returns:**
- Iterator to the found element, or `end()` if not found

**Performance:**
- **Time Complexity:** O(1) average case, O(n) worst case

**Example:**
```cpp
auto it = ages.find("Alice");
if (it != ages.end()) {
    turbokit::log.info("Alice is %d years old", it->second);
}
```

##### `operator[]`

```cpp
ValueType& operator[](const KeyType& key);
ValueType& operator[](KeyType&& key);
```

Returns a reference to the value for the given key, inserting if necessary.

**Parameters:**
- `key`: Key to access

**Returns:**
- Reference to the value associated with the key

**Performance:**
- **Time Complexity:** O(1) average case, O(n) worst case
- **Side Effects:** May insert a default-constructed value

**Example:**
```cpp
turbokit::HashMap<std::string, int> counters;
counters["requests"]++; // Creates entry if doesn't exist
```

##### `at()`

```cpp
ValueType& at(const KeyType& key);
const ValueType& at(const KeyType& key) const;
```

Returns a reference to the value for the given key with bounds checking.

**Parameters:**
- `key`: Key to access

**Returns:**
- Reference to the value associated with the key

**Throws:**
- `std::out_of_range`: If key is not found

**Performance:**
- **Time Complexity:** O(1) average case, O(n) worst case

##### `erase()`

```cpp
size_t erase(const KeyType& key);
iterator erase(iterator position);
```

Removes elements from the hash map.

**Parameters:**
- `key`: Key to remove
- `position`: Iterator to element to remove

**Returns:**
- Number of elements removed (0 or 1 for key version)
- Iterator to the next element (for iterator version)

**Performance:**
- **Time Complexity:** O(1) average case, O(n) worst case

#### Capacity and Size

##### `size()`

```cpp
size_t size() const;
```

Returns the number of elements in the hash map.

**Returns:**
- Number of key-value pairs currently stored

**Performance:**
- **Time Complexity:** O(1)

##### `empty()`

```cpp
bool empty() const;
```

Returns whether the hash map is empty.

**Returns:**
- `true` if size() == 0, `false` otherwise

**Performance:**
- **Time Complexity:** O(1)

##### `bucket_count()`

```cpp
size_t bucket_count() const;
```

Returns the number of buckets in the hash table.

**Returns:**
- Current number of buckets

**Performance:**
- **Time Complexity:** O(1)

##### `load_factor()`

```cpp
double load_factor() const;
```

Returns the current load factor (size / bucket_count).

**Returns:**
- Current load factor

**Performance:**
- **Time Complexity:** O(1)

##### `rehash()`

```cpp
void rehash(size_t new_bucket_count);
```

Changes the number of buckets and rehashes all elements.

**Parameters:**
- `new_bucket_count`: Target number of buckets

**Performance:**
- **Time Complexity:** O(n) where n is `size()`
- **Memory Usage:** Allocates new bucket array

##### `reserve()`

```cpp
void reserve(size_t new_capacity);
```

Reserves space for at least the specified number of elements.

**Parameters:**
- `new_capacity`: Minimum number of elements to accommodate

**Performance:**
- **Time Complexity:** O(n) if rehashing occurs, O(1) otherwise
- **Memory Usage:** May allocate larger bucket array

#### Iteration

##### `begin()` / `end()`

```cpp
iterator begin();
iterator end();
const_iterator begin() const;
const_iterator end() const;
```

Returns iterators for traversing all key-value pairs.

**Returns:**
- `begin()`: Iterator to first element
- `end()`: Iterator one past the last element

**Performance:**
- **Time Complexity:** O(1)

**Example:**
```cpp
for (auto& [key, value] : hash_map) {
    process_pair(key, value);
}
```

##### `clear()`

```cpp
void clear();
```

Removes all elements from the hash map.

**Performance:**
- **Time Complexity:** O(n) where n is `size()`
- **Memory Usage:** Bucket array is retained

## Performance Characteristics

### Time Complexity

| Operation | Average Case | Worst Case | Notes |
|-----------|--------------|------------|-------|
| `insert()` | O(1) | O(n) | Worst case with many collisions |
| `find()` | O(1) | O(n) | Worst case with many collisions |
| `erase()` | O(1) | O(n) | Worst case with many collisions |
| `operator[]` | O(1) | O(n) | May insert default value |
| `at()` | O(1) | O(n) | Throws if key not found |
| `rehash()` | O(n) | O(n) | Always linear in size |

### Hash Quality Requirements

For optimal performance, the hash function should:
- **Distribute uniformly** across the hash space
- **Minimize collisions** for expected key sets
- **Be fast to compute** (O(1) complexity)
- **Be consistent** (same key always hashes to same value)

### Load Factor Management

- **Default load factor:** ~0.75 (configurable)
- **Rehashing threshold:** When load factor exceeds 1.0
- **Growth factor:** 2x bucket count during rehashing
- **Memory overhead:** ~25-33% for optimal performance

### Cache Performance

- **Main table:** Contiguous allocation for good cache locality
- **Collision table:** Separate allocation to minimize main table overhead
- **Key-value layout:** Packed structure for cache efficiency
- **Iteration performance:** Good spatial locality for sequential access

## Use Cases

### 1. High-Performance Caching

```cpp
#include <turbokit/hash_map.h>
#include <turbokit/clock.h>

class HighPerformanceCache {
private:
    turbokit::HashMap<uint64_t, CacheEntry> cache;

public:
    HighPerformanceCache(size_t expected_size) {
        cache.reserve(expected_size);
    }

    bool get(uint64_t key, CacheEntry& entry) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            entry = it->second;
            return true;
        }
        return false;
    }

    void put(uint64_t key, const CacheEntry& entry) {
        cache[key] = entry; // Fast insertion or update
    }

    void clear_cache() {
        cache.clear(); // Retains bucket allocation
    }
};
```

### 2. Real-Time Index Structures

```cpp
#include <turbokit/hash_map.h>

class RealTimeIndex {
private:
    turbokit::HashMap<std::string, uint32_t> symbol_to_id;
    turbokit::Vector<std::string> id_to_symbol;

public:
    uint32_t intern_symbol(const std::string& symbol) {
        auto it = symbol_to_id.find(symbol);
        if (it != symbol_to_id.end()) {
            return it->second;
        }

        uint32_t new_id = id_to_symbol.size();
        symbol_to_id[symbol] = new_id;
        id_to_symbol.append(symbol);
        return new_id;
    }

    const std::string& get_symbol(uint32_t id) const {
        return id_to_symbol[id];
    }

    std::optional<uint32_t> lookup_symbol(const std::string& symbol) const {
        auto it = symbol_to_id.find(symbol);
        return (it != symbol_to_id.end()) ? std::make_optional(it->second) : std::nullopt;
    }
};
```

### 3. Configuration Management

```cpp
#include <turbokit/hash_map.h>

class ConfigManager {
private:
    turbokit::HashMap<std::string, std::string> config_values;

public:
    void load_config(const std::string& filename) {
        config_values.clear();

        // Load configuration from file
        for (const auto& [key, value] : parse_config_file(filename)) {
            config_values[key] = value;
        }
    }

    template<typename T>
    T get_value(const std::string& key, const T& default_value) const {
        auto it = config_values.find(key);
        if (it != config_values.end()) {
            return parse_value<T>(it->second);
        }
        return default_value;
    }

    void set_value(const std::string& key, const std::string& value) {
        config_values[key] = value;
    }

    bool has_key(const std::string& key) const {
        return config_values.find(key) != config_values.end();
    }
};
```

### 4. Performance Monitoring

```cpp
#include <turbokit/hash_map.h>
#include <turbokit/clock.h>

class PerformanceMonitor {
private:
    turbokit::HashMap<std::string, PerformanceStats> metrics;

public:
    void record_timing(const std::string& operation, int64_t duration_ns) {
        auto& stats = metrics[operation]; // Creates if doesn't exist
        stats.count++;
        stats.total_time += duration_ns;
        stats.min_time = std::min(stats.min_time, duration_ns);
        stats.max_time = std::max(stats.max_time, duration_ns);
    }

    void print_statistics() const {
        for (const auto& [operation, stats] : metrics) {
            double avg_time = static_cast<double>(stats.total_time) / stats.count;
            turbokit::log.info("%s: count=%zu, avg=%.2f ns, min=%lld ns, max=%lld ns",
                              operation.c_str(), stats.count, avg_time,
                              stats.min_time, stats.max_time);
        }
    }

    void reset_statistics() {
        metrics.clear();
    }
};
```

## Best Practices

### 1. Pre-allocate for Known Sizes

```cpp
// ❌ Bad: Multiple rehashes
turbokit::HashMap<int, std::string> map;
for (int i = 0; i < 100000; ++i) {
    map[i] = std::to_string(i);
}

// ✅ Good: Single allocation
turbokit::HashMap<int, std::string> map;
map.reserve(100000);
for (int i = 0; i < 100000; ++i) {
    map[i] = std::to_string(i);
}
```

### 2. Use Good Hash Functions

```cpp
// ❌ Bad: Poor hash distribution
struct BadHash {
    size_t operator()(const std::string& s) const {
        return s.length(); // Many collisions!
    }
};

// ✅ Good: Use standard hash or high-quality custom hash
turbokit::HashMap<std::string, int> map; // Uses std::hash<std::string>

// Or custom hash with good distribution
struct GoodHash {
    size_t operator()(const CustomKey& key) const {
        return std::hash<uint64_t>{}(key.hash_code());
    }
};
```

### 3. Minimize Key and Value Sizes

```cpp
// ❌ Bad: Large keys/values
turbokit::HashMap<std::string, LargeObject> map;

// ✅ Good: Use IDs or pointers for large objects
turbokit::HashMap<uint32_t, std::unique_ptr<LargeObject>> map;
// Or use string IDs with interning
turbokit::HashMap<uint32_t, LargeObject> map; // With string->ID mapping
```

### 4. Batch Operations When Possible

```cpp
// ❌ Bad: Individual lookups
for (const auto& key : keys_to_process) {
    auto it = map.find(key);
    if (it != map.end()) {
        process_value(it->second);
    }
}

// ✅ Good: Group related operations
std::vector<typename HashMap::iterator> found_items;
found_items.reserve(keys_to_process.size());

for (const auto& key : keys_to_process) {
    auto it = map.find(key);
    if (it != map.end()) {
        found_items.push_back(it);
    }
}

// Process all found items together
for (auto it : found_items) {
    process_value(it->second);
}
```

## Thread Safety

The HashMap class is **not thread-safe**. External synchronization is required for concurrent access:

```cpp
#include <turbokit/sync.h>

turbokit::HashMap<std::string, int> shared_map;
turbokit::SpinMutex map_mutex;

void thread_safe_insert(const std::string& key, int value) {
    std::lock_guard<turbokit::SpinMutex> lock(map_mutex);
    shared_map[key] = value;
}

int thread_safe_lookup(const std::string& key) {
    std::lock_guard<turbokit::SpinMutex> lock(map_mutex);
    auto it = shared_map.find(key);
    return (it != shared_map.end()) ? it->second : -1;
}
```

## Custom Hash Functions

For custom types, provide a hash function:

```cpp
struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// Custom hash function
struct PointHash {
    size_t operator()(const Point& p) const {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

// Usage
turbokit::HashMap<Point, std::string, PointHash> point_names;
point_names[{10, 20}] = "Origin";
```

## Migration from std::unordered_map

### Basic Replacement

```cpp
// Before
std::unordered_map<std::string, int> map;
map["key"] = 42;
map.insert({"key2", 24});

// After
turbokit::HashMap<std::string, int> map;
map["key"] = 42;
map.insert("key2", 24); // Slightly different syntax
```

### Performance Optimization

```cpp
// std::unordered_map - hidden rehashing
std::unordered_map<int, std::string> map;
for (int i = 0; i < 100000; ++i) {
    map[i] = std::to_string(i);
}

// TurboKit HashMap - explicit control
turbokit::HashMap<int, std::string> map;
map.reserve(100000); // Prevent rehashing
for (int i = 0; i < 100000; ++i) {
    map[i] = std::to_string(i);
}
```

---

**See Also:**
- [Vector API](vector.md) - Dynamic array implementation
- [Serialization API](serialization.md) - For serializing hash maps
- [Performance Guide](../performance.md) - Hash table optimization techniques