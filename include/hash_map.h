#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>

#pragma push_macro("likely")
#pragma push_macro("unlikely")
#undef likely
#undef unlikely
#define likely(x) __builtin_expect(bool(x), 1)
#define unlikely(x) __builtin_expect(bool(x), 0)

#pragma push_macro("assert")
#undef assert
#define assert(x)

namespace turbokit {

static constexpr int8_t invalid_marker = -1;
template <typename T> bool isInvalid(const T &v) {
  return v == (T)invalid_marker;
}

template <typename KeyType, typename ValueType,
          typename HashFunction = std::hash<KeyType>,
          typename EqualityFunction = std::equal_to<KeyType>,
          typename MemoryAllocator = std::allocator<void>>
struct HashMap {
private:
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

  size_t bucket_count = 0;
  size_t element_count = 0;
  MainEntry *main_table = nullptr;
  CollisionEntry *collision_table = nullptr;

public:
  struct iterator {
  private:
  public:
    friend HashMap;
    HashMap *container;
    size_t bucket_index;
    size_t collision_index;
    ValueType *value_ptr;
    mutable std::aligned_storage_t<sizeof(std::pair<KeyType &, ValueType &>),
                                   alignof(std::pair<KeyType &, ValueType &>)>
        temporary_storage;

  public:
    iterator() = default;
    iterator(const HashMap *container, size_t bucket_index,
             size_t collision_index, ValueType *value_ptr)
        : container(const_cast<HashMap *>(container)),
          bucket_index(bucket_index), collision_index(collision_index),
          value_ptr(value_ptr) {}
    iterator(const iterator &other) {
      container = other.container;
      bucket_index = other.bucket_index;
      collision_index = other.collision_index;
      value_ptr = other.value_ptr;
    }
    iterator &operator=(const iterator &other) {
      container = other.container;
      bucket_index = other.bucket_index;
      collision_index = other.collision_index;
      value_ptr = other.value_ptr;
      return *this;
    }

    using T = ValueType;

    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::bidirectional_iterator_tag;

    std::pair<KeyType &, ValueType &> &operator*() const noexcept {
      new (&temporary_storage) std::pair<KeyType &, ValueType &>(
          *(KeyType *)((char *)value_ptr - sizeof(KeyType)), *value_ptr);
      return (std::pair<KeyType &, ValueType &> &)temporary_storage;
    }
    std::pair<KeyType &, ValueType &> *operator->() const noexcept {
      return &**this;
    }

    iterator &operator++() noexcept {
      if (isInvalid(collision_index)) {
        do {
          if (bucket_index == container->bucket_count - 1) {
            bucket_index = invalid_marker;
            collision_index = 0;
            value_ptr = &container->collision_table[collision_index].value;
            if (!isInvalid(
                    container->collision_table[collision_index].main_index)) {
              return *this;
            }
            return ++*this;
          } else {
            ++bucket_index;
            value_ptr = &container->main_table[bucket_index].value;
          }
        } while (isInvalid(container->main_table[bucket_index].chain_length));
      } else {
        do {
          if (collision_index == container->bucket_count - 1) {
            value_ptr = nullptr;
            return *this;
          }
          ++collision_index;
          value_ptr = &container->collision_table[collision_index].value;
        } while (
            isInvalid(container->collision_table[collision_index].main_index));
      }
      return *this;
    }
    iterator operator++(int) noexcept {
      iterator result = *this;
      ++result;
      return result;
    }
    bool operator==(iterator other) const noexcept {
      return value_ptr == other.value_ptr;
    }
    bool operator!=(iterator other) const noexcept {
      return value_ptr != other.value_ptr;
    }
  };

  HashMap() = default;
  ~HashMap() {
    clear();
    deallocate(main_table, bucket_count);
    deallocate(collision_table, bucket_count);
  }

  HashMap(const HashMap &other) { *this = other; }

  HashMap &operator=(const HashMap &other) {
    clear();
    for (auto &entry : other) {
      insert(entry);
    }
    return *this;
  }

  bool empty() const noexcept { return element_count == 0; }

  void clear() noexcept {
    auto iter = begin();
    auto end_iter = end();
    while (iter != end_iter) {
      iter = remove(iter);
    }
    assert(element_count == 0);
    assert(begin() == end());
  }
  iterator begin() const noexcept {
    if (unlikely(!main_table)) {
      return end();
    }
    size_t bucket_limit = bucket_count;
    for (size_t i = 0; i != bucket_limit; ++i) {
      if (!isInvalid(main_table[i].chain_length)) {
        return iterator(this, i, invalid_marker, &main_table[i].value);
      }
    }
    for (size_t i = 0; i != bucket_limit; ++i) {
      if (!isInvalid(collision_table[i].main_index)) {
        return iterator(this, i, i, &collision_table[i].value);
      }
    }
    return end();
  }
  iterator end() const noexcept { return iterator(this, 0, 0, nullptr); }

  template <typename T> T *allocate(size_t n) {
    using A = typename std::allocator_traits<
        MemoryAllocator>::template rebind_alloc<T>;
    T *result = A().allocate(n);
    return result;
  }
  template <typename T> void deallocate(T *ptr, size_t n) {
    using A = typename std::allocator_traits<
        MemoryAllocator>::template rebind_alloc<T>;
    if (ptr) {
      A().deallocate(ptr, n);
    }
  }

  template <typename KeyT> void remove(KeyT &&key) {
    auto iter = find(std::forward<KeyT>(key));
    if (iter != end()) {
      remove(iter);
    }
  }

  template <typename KeyT> ValueType &operator[](KeyT &&index) {
    return try_insert(std::forward<KeyT>(index)).first->second;
  }

  iterator remove(iterator iter) noexcept {
    auto mask = bucket_count - 1;
    size_t bucket_idx = iter.bucket_index;
    size_t collision_idx = iter.collision_index;
    assert(iter.value_ptr);
    assert(bucket_idx < bucket_count);
    assert(isInvalid(iter.collision_index) || collision_idx < bucket_count);
    assert(element_count > 0);
    --element_count;
    auto *main_table = this->main_table;
    auto *collision_table = this->collision_table;
    if (likely(isInvalid(collision_idx))) {
      auto &main_entry = main_table[bucket_idx];
      size_t chain_len = main_entry.chain_length;
      if (unlikely(chain_len)) {
        --chain_len;
        size_t index = (bucket_idx + chain_len) & mask;
        auto &collision_entry = collision_table[index];
        assert(collision_entry.main_index == bucket_idx);
        collision_entry.main_index = invalid_marker;
        main_entry.key = std::move(collision_entry.key);
        main_entry.value = std::move(collision_entry.value);
        collision_entry.key.~KeyType();
        collision_entry.value.~ValueType();
        while (
            unlikely(chain_len) &&
            collision_table[(bucket_idx + chain_len - 1) & mask].main_index !=
                bucket_idx) {
          --chain_len;
        }
        main_entry.chain_length = chain_len;
        return iter;
      }
      ++iter;
      main_entry.chain_length = invalid_marker;
      main_entry.key.~KeyType();
      main_entry.value.~ValueType();
      assert(
          iter == end() ||
          (isInvalid(iter.collision_index)
               ? !isInvalid(main_table[iter.bucket_index].chain_length)
               : !isInvalid(collision_table[iter.collision_index].main_index)));
      return iter;
    } else {
      auto &collision_entry = collision_table[collision_idx];
      if (isInvalid(bucket_idx)) {
        bucket_idx = collision_entry.main_index;
      }
      auto &main_entry = main_table[bucket_idx];
      size_t chain_len = main_entry.chain_length - 1;
      assert(main_entry.chain_length > 0 &&
             ((collision_idx - bucket_idx) & mask) <= chain_len);
      size_t last_index = (bucket_idx + chain_len) & mask;
      auto &last_entry = collision_table[last_index];
      assert(last_entry.main_index == bucket_idx);
      if (last_index == collision_idx) {
        ++iter;
      } else {
        collision_entry.key = std::move(last_entry.key);
        collision_entry.value = std::move(last_entry.value);
      }
      last_entry.main_index = invalid_marker;
      last_entry.key.~KeyType();
      last_entry.value.~ValueType();
      while (chain_len &&
             collision_table[(bucket_idx + chain_len - 1) & mask].main_index !=
                 bucket_idx) {
        --chain_len;
      }
      main_entry.chain_length = chain_len;
      assert(
          iter == end() ||
          (isInvalid(iter.collision_index)
               ? !isInvalid(main_table[iter.bucket_index].chain_length)
               : !isInvalid(collision_table[iter.collision_index].main_index)));
      return iter;
    }
  }

  template <typename KeyT, typename ValueT>
  iterator insert(KeyT &&key, ValueT &&value) {
    return try_insert(std::forward<KeyType>(key),
                      std::forward<ValueType>(value))
        .first;
  }

  void resize_table(size_t new_bucket_count) noexcept {
    if (new_bucket_count & (new_bucket_count - 1)) {
      printf("bucket count is not a multiple of 2!\n");
      std::abort();
    }
    MainEntry *old_main_table = main_table;
    CollisionEntry *old_collision_table = collision_table;

    main_table = allocate<MainEntry>(new_bucket_count);
    collision_table = allocate<CollisionEntry>(new_bucket_count);

    size_t old_bucket_count = bucket_count;

    for (size_t i = 0; i != new_bucket_count; ++i) {
      main_table[i].chain_length = invalid_marker;
      collision_table[i].main_index = invalid_marker;
    }

    bucket_count = new_bucket_count;
    element_count = 0;

    if (old_main_table) {
      MainEntry *main_end = old_main_table + old_bucket_count;
      for (auto *iter = old_main_table; iter != main_end; ++iter) {
        if (!isInvalid(iter->chain_length)) {
          try_insert(std::move(iter->key), std::move(iter->value));
          iter->key.~KeyType();
          iter->value.~ValueType();
        }
      }
      CollisionEntry *collision_end = old_collision_table + old_bucket_count;
      for (auto *iter = old_collision_table; iter != collision_end; ++iter) {
        if (!isInvalid(iter->main_index)) {
          try_insert(std::move(iter->key), std::move(iter->value));
          iter->key.~KeyType();
          iter->value.~ValueType();
        }
      }
    }

    deallocate(old_main_table, old_bucket_count);
    deallocate(old_collision_table, old_bucket_count);
  }

  template <typename KeyT>
  iterator find_in_collision_chain(size_t bucket_idx,
                                   KeyT &&key) const noexcept {
    size_t bucket_limit = bucket_count;
    size_t mask = bucket_limit - 1;
    auto *main_table = this->main_table;
    auto *collision_table = this->collision_table;
    auto &main_entry = main_table[bucket_idx];
    auto equal = [](CollisionEntry *entry, size_t bucket_idx, auto &&key) {
      if constexpr (std::is_trivial_v<KeyT> && std::is_trivial_v<KeyType>) {
        return (bool)(entry->main_index == bucket_idx & entry->key == key);
      }
      return entry->main_index == bucket_idx &&
             EqualityFunction()(entry->key, key);
    };

    size_t chain_len = main_entry.chain_length;
    size_t end_collision_idx = (bucket_idx + chain_len) & mask;
    iterator end_iterator(this, bucket_idx, end_collision_idx, nullptr);
    size_t collision_idx = bucket_idx;
    CollisionEntry *begin_ptr = collision_table + collision_idx;
    CollisionEntry *end_ptr = collision_table + end_collision_idx;
    CollisionEntry *current_ptr = begin_ptr;
    CollisionEntry *wrap_ptr = collision_table + bucket_limit;
    do {
      if (likely(end_ptr > current_ptr)) {
        switch (chain_len) {
        default:
        case 4:
          if (equal(current_ptr, bucket_idx, key)) {
            return iterator(this, bucket_idx, collision_idx & mask,
                            &current_ptr->value);
          }
          ++current_ptr;
          ++collision_idx;
          --chain_len;
        case 3:
          if (equal(current_ptr, bucket_idx, key)) {
            return iterator(this, bucket_idx, collision_idx & mask,
                            &current_ptr->value);
          }
          ++current_ptr;
          ++collision_idx;
          --chain_len;
        case 2:
          if (equal(current_ptr, bucket_idx, key)) {
            return iterator(this, bucket_idx, collision_idx & mask,
                            &current_ptr->value);
          }
          ++current_ptr;
          ++collision_idx;
          --chain_len;
        case 1:
          if (equal(current_ptr, bucket_idx, key)) {
            return iterator(this, bucket_idx, collision_idx & mask,
                            &current_ptr->value);
          }
          ++current_ptr;
          ++collision_idx;
          --chain_len;
          if (current_ptr == end_ptr) {
            return end_iterator;
          }
        }
      }
      if (equal(current_ptr, bucket_idx, key)) {
        return iterator(this, bucket_idx, collision_idx & mask,
                        &current_ptr->value);
      }
      ++current_ptr;
      ++collision_idx;
      --chain_len;
      if (current_ptr == wrap_ptr) {
        current_ptr = collision_table;
      }
    } while (current_ptr != end_ptr);
    return end_iterator;
  }

  template <typename KeyT> iterator find(KeyT &&key) const noexcept {
    if (unlikely(!main_table)) {
      return end();
    }
    size_t bucket_limit = bucket_count;
    size_t mask = bucket_limit - 1;
    size_t bucket_idx = HashFunction()(key) & mask;

    auto *main_table = this->main_table;

    auto &main_entry = main_table[bucket_idx];

    if (isInvalid(main_entry.chain_length)) {
      return iterator(this, bucket_idx, invalid_marker, nullptr);
    }
    if (likely(EqualityFunction()(main_entry.key, key))) {
      return iterator(this, bucket_idx, invalid_marker, &main_entry.value);
    }
    if (likely(main_entry.chain_length == 0)) {
      return iterator(this, bucket_idx, bucket_idx, nullptr);
    }
    return find_in_collision_chain(bucket_idx, std::forward<KeyT>(key));
  }

  void reserve(size_t n) {
    if (n >= std::numeric_limits<size_t>::max() / 2) {
      throw std::range_error("reserve beyond max size");
    }
    size_t bucket_limit = bucket_count;
    if (bucket_limit == 0) {
      bucket_limit = 1;
    }
    while (bucket_limit < n) {
      bucket_limit *= 2;
    }
    resize_table(bucket_limit);
  }

  template <typename KeyT, typename... Args>
  std::pair<iterator, bool> try_insert(KeyT &&key, Args &&...args) {
    if (unlikely(!main_table)) {
      reserve(16);
    }
    auto iter = find(key);
    if (unlikely(iter.value_ptr)) {
      return std::make_pair(iter, false);
    }
    size_t new_bucket_count = element_count + 1;
    if (unlikely(bucket_count < new_bucket_count)) {
    resize:
      reserve(new_bucket_count);
      iter = find(key);
      assert(iter.value_ptr == nullptr);
    }
    auto *main_table = this->main_table;
    auto &main_entry = main_table[iter.bucket_index];
    if (likely(isInvalid(iter.collision_index))) {
      ++element_count;
      main_entry.chain_length = 0;
      new (&main_entry.key) KeyType(std::forward<KeyT>(key));
      new (&main_entry.value) ValueType(std::forward<Args>(args)...);
      return std::make_pair(
          iterator(this, iter.bucket_index, invalid_marker, &main_entry.value),
          true);
    } else if (bucket_count < 1024 * 1024 / sizeof(MainEntry) &&
               bucket_count < element_count * 4) {
    expand:
      new_bucket_count = bucket_count + 1;
      goto resize;
    } else {
      size_t mask = bucket_count - 1;
      size_t chain_len = main_entry.chain_length + 1;
      size_t index = iter.collision_index;
      while (!isInvalid(collision_table[index].main_index)) {
        index = (index + 1) & mask;
        ++chain_len;
      }
      if ((unlikely(chain_len >= 4) &&
           bucket_count < 1024 * 1024 * 4 / sizeof(MainEntry)) ||
          (unlikely(chain_len >= bucket_count / 2))) {
        goto expand;
      }
      ++element_count;
      main_entry.chain_length = chain_len;
      auto &collision_entry = collision_table[index];
      collision_entry.main_index = iter.bucket_index;
      new (&collision_entry.key) KeyType(std::forward<KeyT>(key));
      new (&collision_entry.value) ValueType(std::forward<Args>(args)...);
      return std::make_pair(
          iterator(this, iter.bucket_index, index, &collision_entry.value),
          true);
    }
  }

  template <typename... Args> auto emplace(Args &&...args) {
    return try_insert(std::forward<Args>(args)...);
  }

  size_t get_bucket_count() const noexcept { return bucket_count; }
  size_t size() const noexcept { return element_count; }

  auto insert(const std::pair<KeyType, ValueType> &x) {
    return emplace(x.first, x.second);
  }
};

#pragma pop_macro("assert")
#pragma pop_macro("likely")
#pragma pop_macro("unlikely")

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Allocator = std::allocator<void>>
using HashMap = HashMap<Key, Value, Hash, Equal, Allocator>;

} // namespace turbokit
