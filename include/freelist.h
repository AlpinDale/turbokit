#pragma once

#include "sync.h"

#include <unordered_set>
#include <utility>
#include <vector>

namespace turbokit {

template <typename ElementType> struct ThreadLocalPool {
  ElementType *first_element = nullptr;
  size_t element_count = 0;
  static ThreadLocalPool &get_instance() {
    thread_local ThreadLocalPool pool;
    return pool;
  }
};

template <typename ElementType> struct SharedPool {
  SpinMutex synchronization_lock;
  std::vector<std::pair<ElementType *, size_t>> available_elements;
  static SharedPool &get_instance() {
    static SharedPool pool;
    return pool;
  }
};

template <typename ElementType> struct MemoryPool {
  template <typename StorageType> static auto read_value(StorageType &storage) {
    if constexpr (std::is_scalar_v<StorageType>) {
      return storage;
    } else {
      return storage.load(std::memory_order_relaxed);
    }
  }

  template <typename DestinationType, typename SourceType>
  static void write_value(DestinationType &destination, SourceType value) {
    if constexpr (std::is_scalar_v<DestinationType>) {
      destination = value;
    } else {
      destination.store(value, std::memory_order_relaxed);
    }
  }

  [[gnu::always_inline]] static void add_element(ElementType *element,
                                                 size_t max_local_elements) {
    auto &local_pool = ThreadLocalPool<ElementType>::get_instance();
    if (local_pool.element_count == max_local_elements) {
      [[unlikely]];
      transfer_to_shared_pool(local_pool, max_local_elements / 8u);
    }
    [[likely]];
    ++local_pool.element_count;
    ElementType *previous_first =
        std::exchange(local_pool.first_element, element);
    write_value(element->next, previous_first);
  }

  [[gnu::always_inline]] static ElementType *remove_element() {
    auto &local_pool = ThreadLocalPool<ElementType>::get_instance();
    ElementType *result = local_pool.first_element;
    if (result) {
      [[likely]];
      --local_pool.element_count;
      local_pool.first_element = (ElementType *)read_value(result->next);
      return result;
    }
    [[unlikely]];
    return remove_from_shared_pool(local_pool);
  }

  [[gnu::noinline]] static ElementType *
  remove_from_shared_pool(ThreadLocalPool<ElementType> &local_pool) {
    auto &shared_pool = SharedPool<ElementType>::get_instance();
    std::unique_lock lock(shared_pool.synchronization_lock);
    if (shared_pool.available_elements.empty()) {
      [[unlikely]];
      return nullptr;
    }
    ElementType *result;
    size_t remaining_count;
    std::tie(result, remaining_count) = shared_pool.available_elements.back();
    shared_pool.available_elements.pop_back();
    lock.unlock();
    local_pool.first_element = (ElementType *)read_value(result->next);
    local_pool.element_count = remaining_count - 1;
    return result;
  }

  [[gnu::noinline]] static void
  transfer_to_shared_pool(ThreadLocalPool<ElementType> &local_pool,
                          size_t elements_to_retain) {
    ElementType *current = local_pool.first_element;
    size_t new_count = 1;
    while (new_count < elements_to_retain) {
      ElementType *next_element = (ElementType *)read_value(current->next);
      current = next_element;
      ++new_count;
    }
    size_t original_count = local_pool.element_count;
    local_pool.element_count = new_count;
    ElementType *next_element = (ElementType *)read_value(current->next);
    write_value(current->next, nullptr);
    auto &shared_pool = SharedPool<ElementType>::get_instance();
    std::lock_guard lock(shared_pool.synchronization_lock);
    shared_pool.available_elements.emplace_back(next_element,
                                                original_count - new_count);
  }
};

template <typename T> using FreeListTlsStorage = ThreadLocalPool<T>;

template <typename T> using FreeListGlobalStorage = SharedPool<T>;

template <typename T> using FreeList = MemoryPool<T>;

} // namespace turbokit
