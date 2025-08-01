#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace turbokit {

template <typename ElementType,
          typename MemoryManager = std::allocator<ElementType>>
class DynamicArray {
private:
  ElementType *memory_start = nullptr;
  ElementType *memory_limit = nullptr;
  ElementType *current_start = nullptr;
  ElementType *current_end = nullptr;
  size_t element_count = 0;

  using value_type = ElementType;

public:
  DynamicArray() = default;

  DynamicArray(const DynamicArray &other) { *this = other; }

  DynamicArray(DynamicArray &&other) noexcept { *this = std::move(other); }

  explicit DynamicArray(size_t initial_size) { resize(initial_size); }

  ~DynamicArray() {
    if (current_start != current_end) {
      clear();
    }
    if (memory_start) {
      MemoryManager().deallocate(memory_start, memory_limit - memory_start);
    }
  }

  DynamicArray &operator=(const DynamicArray &other) {
    clear();
    reserve(other.size());
    size_t count = other.size();
    for (size_t i = 0; i != count; ++i) {
      new (current_end) ElementType(other[i]);
      ++current_end;
      ++element_count;
    }
    return *this;
  }

  DynamicArray &operator=(DynamicArray &&other) noexcept {
    std::swap(memory_start, other.memory_start);
    std::swap(memory_limit, other.memory_limit);
    std::swap(current_start, other.current_start);
    std::swap(current_end, other.current_end);
    std::swap(element_count, other.element_count);
    return *this;
  }

  ElementType &get_at(size_t position) {
    if (position >= element_count) {
      throw std::out_of_range("DynamicArray::get_at out of range");
    }
    return current_start[position];
  }

  const ElementType &get_at(size_t position) const {
    if (position >= element_count) {
      throw std::out_of_range("DynamicArray::get_at out of range");
    }
    return current_start[position];
  }

  size_t size() const { return element_count; }

  ElementType *get_data() { return current_start; }

  const ElementType *get_data() const { return current_start; }

  ElementType *get_begin() { return current_start; }

  ElementType *get_end() { return current_end; }

  const ElementType *get_begin() const { return current_start; }

  const ElementType *get_end() const { return current_end; }

  // Range-based for loop support
  ElementType *begin() { return current_start; }

  ElementType *end() { return current_end; }

  const ElementType *begin() const { return current_start; }

  const ElementType *end() const { return current_end; }

  ElementType &operator[](size_t position) {
    if (position >= element_count) {
      throw std::out_of_range("DynamicArray::operator[] out of range");
    }
    return current_start[position];
  }

  const ElementType &operator[](size_t position) const {
    if (position >= element_count) {
      throw std::out_of_range("DynamicArray::operator[] out of range");
    }
    return current_start[position];
  }

  void clear() {
    for (auto *ptr = current_start; ptr != current_end; ++ptr) {
      ptr->~ElementType();
    }
    current_start = memory_start;
    current_end = current_start;
    element_count = 0;
  }

  void relocate_elements(ElementType *destination, ElementType *source_begin,
                         ElementType *source_end) {
    if constexpr (std::is_trivially_copyable_v<ElementType>) {
      std::memmove((void *)destination, (void *)source_begin,
                   (source_end - source_begin) * sizeof(ElementType));
    } else {
      if (destination <= source_begin) {
        for (auto *ptr = source_begin; ptr != source_end;) {
          *destination = std::move(*ptr);
          ++destination;
          ++ptr;
        }
      } else {
        auto *dest_end = destination + (source_end - source_begin);
        for (auto *ptr = source_end; ptr != source_begin;) {
          --dest_end;
          --ptr;
          *dest_end = std::move(*ptr);
        }
      }
    }
  }

  void remove_range(ElementType *range_begin, ElementType *range_end) {
    size_t removed_count = range_end - range_begin;
    element_count -= removed_count;

    if (range_begin == current_start) {
      for (auto *ptr = range_begin; ptr != range_end; ++ptr) {
        ptr->~ElementType();
      }
      current_start = range_end;

      if (current_start != current_end) {
        size_t unused_space = current_start - memory_start;
        if (unused_space > element_count &&
            unused_space >= 1024 * 512 / sizeof(ElementType)) {
          if constexpr (std::is_trivially_copyable_v<ElementType>) {
            relocate_elements(memory_start, current_start, current_end);
          } else {
            auto *mem_ptr = memory_start;
            auto *curr_ptr = current_start;
            while (mem_ptr != current_start && curr_ptr != current_end) {
              new (mem_ptr) ElementType(std::move(*curr_ptr));
              ++mem_ptr;
              ++curr_ptr;
            }
            relocate_elements(mem_ptr, curr_ptr, current_end);
            for (auto *ptr = curr_ptr; ptr != current_end; ++ptr) {
              ptr->~ElementType();
            }
          }
          current_start = memory_start;
          current_end = current_start + element_count;
        }
      }
    } else {
      relocate_elements(range_begin, range_end, current_end);
      for (auto *ptr = current_end - removed_count; ptr != current_end; ++ptr) {
        ptr->~ElementType();
      }
      current_end -= removed_count;
    }

    if (current_start == current_end) {
      current_start = memory_start;
      current_end = current_start;
    }
  }

  ElementType *remove_at(ElementType *position) {
    size_t index = position - current_start;
    if (index >= element_count) {
      throw std::out_of_range("DynamicArray::remove_at out of range");
    }
    remove_range(position, position + 1);
    return current_start + index;
  }

  void resize(size_t new_size) {
    if (element_count > new_size) {
      ElementType *ptr = current_end;
      ElementType *end_ptr = current_start + new_size;
      while (ptr != end_ptr) {
        --ptr;
        ptr->~ElementType();
      }
    } else if (new_size > element_count) {
      reserve(new_size);
      ElementType *ptr = current_end;
      ElementType *end_ptr = current_start + new_size;
      while (ptr != end_ptr) {
        new (ptr) ElementType();
        ++ptr;
      }
    }
    current_end = current_start + new_size;
    element_count = new_size;
  }

  bool is_empty() const { return current_start == current_end; }

  size_t get_capacity() { return memory_limit - current_start; }

  void allocate_memory(size_t required_size) {
    auto *old_start = current_start;
    auto *old_end = current_end;
    auto *old_memory = memory_start;
    size_t old_count = element_count;

    ElementType *new_memory = MemoryManager().allocate(required_size);

    if (old_memory) {
      if constexpr (std::is_trivially_copyable_v<ElementType>) {
        std::memcpy(new_memory, old_start, sizeof(ElementType) * old_count);
      } else {
        ElementType *dest = new_memory;
        for (ElementType *ptr = old_start; ptr != old_end; ++ptr) {
          new (dest) ElementType(std::move(*ptr));
          ptr->~ElementType();
          ++dest;
        }
      }
      MemoryManager().deallocate(old_memory, memory_limit - old_memory);
    }

    memory_start = new_memory;
    memory_limit = new_memory + required_size;
    current_start = new_memory;
    current_end = new_memory + old_count;
  }

  void reserve(size_t required_size) {
    if (required_size <= get_capacity()) {
      return;
    }
    allocate_memory(required_size);
  }

  void grow_capacity() {
    allocate_memory(std::max(get_capacity() * 2, (size_t)16));
  }

  void append(const ElementType &element) { emplace_back(element); }

  void append(ElementType &&element) { emplace_back(std::move(element)); }

  template <typename... ConstructorArgs>
  void emplace_back(ConstructorArgs &&...args) {
    if (current_end == memory_limit) {
      if (get_capacity() != size()) {
        __builtin_unreachable();
      }
      [[unlikely]];
      grow_capacity();
    }
    new (current_end) ElementType(std::forward<ConstructorArgs>(args)...);
    ++current_end;
    ++element_count;
  }

  ElementType &get_first() {
    if (element_count == 0) {
      throw std::out_of_range("DynamicArray::get_first called on empty array");
    }
    return *current_start;
  }

  ElementType &get_last() {
    if (element_count == 0) {
      throw std::out_of_range("DynamicArray::get_last called on empty array");
    }
    return current_end[-1];
  }

  void remove_last() {
    if (element_count == 0) {
      throw std::out_of_range(
          "DynamicArray::remove_last called on empty array");
    }
    --current_end;
    --element_count;
    current_end->~ElementType();
  }

  void remove_first() {
    if (element_count == 0) {
      throw std::out_of_range(
          "DynamicArray::remove_first called on empty array");
    }
    remove_range(current_start, current_start + 1);
  }

  ElementType *insert_at(ElementType *position, const ElementType &value) {
    if (position == current_end) {
      append(value);
      return &get_last();
    }

    if (current_end == memory_limit) {
      if (get_capacity() != size()) {
        __builtin_unreachable();
      }
      [[unlikely]];
      size_t index = position - current_start;
      grow_capacity();
      position = current_start + index;
    }

    new (current_end) ElementType(std::move(current_end[-1]));
    relocate_elements(position + 1, position, current_end);
    *position = value;
    ++current_end;
    ++element_count;
    return position;
  }
};

template <typename T, typename Allocator = std::allocator<T>>
using Vector = DynamicArray<T, Allocator>;

} // namespace turbokit
