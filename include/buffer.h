#pragma once

#include "vector.h"

#include <atomic>

namespace turbokit {

struct alignas(std::max_align_t) MemoryBlock {
  union {
    MemoryBlock *successor;
    size_t capacity;
  };
  std::atomic_uint32_t reference_count;

  std::byte *get_data() { return (std::byte *)(this + 1); }

  const std::byte *get_data() const { return (const std::byte *)(this + 1); }

  size_t get_size() const { return capacity; }

  MemoryBlock() = delete;
  ~MemoryBlock() = delete;

  static MemoryBlock *create(size_t bytes_needed) {
    MemoryBlock *result =
        (MemoryBlock *)std::malloc(sizeof(MemoryBlock) + bytes_needed);
    if (!result) {
      throw std::bad_alloc();
    }
    result->capacity = bytes_needed;
    result->reference_count.store(0, std::memory_order_relaxed);
    return result;
  }

  static void destroy(MemoryBlock *block) { std::free((std::byte *)block); }
};

static_assert(std::is_trivial_v<MemoryBlock>);

class UniqueMemoryBlock {
private:
  MemoryBlock *block_ptr = nullptr;

public:
  UniqueMemoryBlock() = default;

  UniqueMemoryBlock(std::nullptr_t) noexcept {}

  explicit UniqueMemoryBlock(MemoryBlock *block) noexcept : block_ptr(block) {}

  UniqueMemoryBlock(const UniqueMemoryBlock &) = delete;

  UniqueMemoryBlock &operator=(const UniqueMemoryBlock &) = delete;

  UniqueMemoryBlock(UniqueMemoryBlock &&other) noexcept {
    block_ptr = other.block_ptr;
    other.block_ptr = nullptr;
  }

  UniqueMemoryBlock &operator=(UniqueMemoryBlock &&other) noexcept {
    std::swap(block_ptr, other.block_ptr);
    return *this;
  }

  ~UniqueMemoryBlock() {
    if (block_ptr) {
      MemoryBlock::destroy(block_ptr);
      block_ptr = nullptr;
    }
  }

  explicit operator bool() const noexcept { return block_ptr; }

  MemoryBlock *operator->() const noexcept { return block_ptr; }

  operator MemoryBlock *() const noexcept { return block_ptr; }

  MemoryBlock *relinquish() noexcept {
    MemoryBlock *result = block_ptr;
    block_ptr = nullptr;
    return result;
  }
};

class SharedMemoryBlock {
private:
  MemoryBlock *block_ptr = nullptr;

public:
  SharedMemoryBlock() = default;

  SharedMemoryBlock(std::nullptr_t) noexcept {}

  explicit SharedMemoryBlock(MemoryBlock *block) : block_ptr(block) {
    if (block) {
      if (block->reference_count.load(std::memory_order_relaxed) != 0) {
        throw std::runtime_error("SharedMemoryBlock: block reference count "
                                 "must be 0 before taking ownership");
      }
      increment_reference();
    }
  }

  SharedMemoryBlock(const SharedMemoryBlock &other) noexcept {
    block_ptr = other.block_ptr;
    if (block_ptr) {
      increment_reference();
    }
  }

  SharedMemoryBlock &operator=(const SharedMemoryBlock &other) noexcept {
    block_ptr = other.block_ptr;
    if (block_ptr) {
      increment_reference();
    }
    return *this;
  }

  SharedMemoryBlock(SharedMemoryBlock &&other) noexcept {
    block_ptr = other.block_ptr;
    other.block_ptr = nullptr;
  }

  SharedMemoryBlock &operator=(SharedMemoryBlock &&other) noexcept {
    std::swap(block_ptr, other.block_ptr);
    return *this;
  }

  ~SharedMemoryBlock() {
    if (block_ptr && decrement_reference() == 0) {
      MemoryBlock::destroy(block_ptr);
    }
  }

  explicit operator bool() const noexcept { return block_ptr; }

  MemoryBlock *operator->() const noexcept { return block_ptr; }

  operator MemoryBlock *() const noexcept { return block_ptr; }

  int increment_reference() noexcept {
    return block_ptr->reference_count.fetch_add(1, std::memory_order_acquire) +
           1;
  }

  int decrement_reference() noexcept {
    return block_ptr->reference_count.fetch_sub(1) - 1;
  }

  MemoryBlock *relinquish() noexcept {
    MemoryBlock *result = block_ptr;
    block_ptr = nullptr;
    return result;
  }

  void take_ownership(MemoryBlock *block) noexcept { block_ptr = block; }
};

inline UniqueMemoryBlock createMemoryBlock(size_t bytes_needed) {
  return UniqueMemoryBlock(MemoryBlock::create(bytes_needed));
}

using Buffer = MemoryBlock;
using BufferHandle = UniqueMemoryBlock;
using SharedBufferHandle = SharedMemoryBlock;

inline BufferHandle makeBuffer(size_t nbytes) {
  return BufferHandle(Buffer::create(nbytes));
}

} // namespace turbokit
