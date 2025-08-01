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
          typename MemoryAllocator = std::allocator<ElementType>>
struct BasicArray {
  ElementType *elements = nullptr;
  size_t count = 0;
  using value_type = ElementType;
  BasicArray() = default;
  BasicArray(const BasicArray &other) { *this = other; }
  BasicArray(BasicArray &&other) { *this = std::move(other); }
  ~BasicArray() { clear(); }
  BasicArray(std::initializer_list<ElementType> list) {
    resize(list.size());
    size_t index = 0;
    for (auto &element : list) {
      (*this)[index] = std::move(element);
      ++index;
    }
  }
  BasicArray(size_t size) { resize(size); }
  BasicArray &operator=(const BasicArray &other) {
    resize(other.count);
    for (size_t index = count; index;) {
      --index;
      elements[index] = other.elements[index];
    }
    return *this;
  }
  BasicArray &operator=(BasicArray &&other) noexcept {
    std::swap(elements, other.elements);
    std::swap(count, other.count);
    return *this;
  }
  ElementType &get_at(size_t position) {
    if (position >= count) {
      throw std::out_of_range("BasicArray::get_at out of range");
    }
    return elements[position];
  }
  const ElementType &get_at(size_t position) const {
    if (position >= count) {
      throw std::out_of_range("BasicArray::get_at out of range");
    }
    return elements[position];
  }
  size_t size() const { return count; }
  ElementType *get_data() { return elements; }
  const ElementType *get_data() const { return elements; }
  ElementType *get_begin() { return elements; }
  ElementType *get_end() { return elements + count; }
  const ElementType *get_begin() const { return elements; }
  const ElementType *get_end() const { return elements + count; }
  ElementType &operator[](size_t position) { return elements[position]; }
  const ElementType &operator[](size_t position) const {
    return elements[position];
  }
  void clear() {
    if (count) {
      for (size_t index = count; index;) {
        --index;
        elements[index].~ElementType();
      }
      MemoryAllocator().deallocate(elements, count);
      count = 0;
    }
  }
  void relocate_elements(ElementType *destination, ElementType *source_begin,
                         ElementType *source_end) noexcept {
    if constexpr (std::is_trivially_copyable_v<ElementType>) {
      std::memmove((void *)destination, (void *)source_begin,
                   (source_end - source_begin) * sizeof(ElementType));
    } else {
      if (destination <= source_begin) {
        for (auto *current = source_begin; current != source_end;) {
          *destination = std::move(*current);
          ++destination;
          ++current;
        }
      } else {
        auto *dest_end = destination + (source_end - source_begin);
        for (auto *current = source_end; current != source_begin;) {
          --dest_end;
          --current;
          *dest_end = std::move(*current);
        }
      }
    }
  }
  void resize(size_t new_size) {
    if (count > new_size) {
      ElementType *current = elements + count;
      ElementType *end_ptr = elements + new_size;
      while (current != end_ptr) {
        --current;
        current->~ElementType();
      }
    } else if (new_size > count) {
      ElementType *new_elements = MemoryAllocator().allocate(new_size);
      relocate_elements(new_elements, elements, elements + count);
      MemoryAllocator().deallocate(elements, count);
      elements = new_elements;
      ElementType *current = elements + count;
      ElementType *end_ptr = elements + new_size;
      while (current != end_ptr) {
        try {
          new (current) ElementType();
        } catch (...) {
          count = current - elements;
          throw;
        }
        ++current;
      }
    }
    count = new_size;
  }
  bool is_empty() const { return count == 0; }
  ElementType &get_first() { return *elements; }
  ElementType &get_last() { return elements[count - 1]; }
};

template <typename T, typename Allocator = std::allocator<T>>
using SimpleVector = BasicArray<T, Allocator>;

} // namespace turbokit
