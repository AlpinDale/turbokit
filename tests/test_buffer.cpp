#include "buffer.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace turbokit;

class BufferTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(BufferTest, BufferAllocation) {
  const size_t size = 1024;
  Buffer *buffer = createMemoryBlock(size);

  EXPECT_NE(buffer, nullptr);
  EXPECT_EQ(buffer->get_size(), size);
  EXPECT_EQ(buffer->reference_count.load(), 0);

  Buffer::destroy(buffer);
}

TEST_F(BufferTest, BufferDataAccess) {
  const size_t size = 100;
  Buffer *buffer = createMemoryBlock(size);

  std::byte *data = buffer->get_data();
  EXPECT_NE(data, nullptr);

  // Test writing and reading data
  for (size_t i = 0; i < size; ++i) {
    data[i] = std::byte(i % 256);
  }

  for (size_t i = 0; i < size; ++i) {
    EXPECT_EQ(data[i], std::byte(i % 256));
  }

  Buffer::destroy(buffer);
}

TEST_F(BufferTest, BufferHandleDefaultConstruction) {
  BufferHandle handle;
  EXPECT_FALSE(handle);
}

TEST_F(BufferTest, BufferHandleExplicitConstruction) {
  const size_t size = 512;
  Buffer *buffer = createMemoryBlock(size);

  BufferHandle handle(buffer);
  EXPECT_TRUE(handle);
  EXPECT_EQ(handle->get_size(), size);

  // handle will automatically deallocate buffer when it goes out of scope
}

TEST_F(BufferTest, BufferHandleMoveConstruction) {
  const size_t size = 256;
  Buffer *buffer = createMemoryBlock(size);
  BufferHandle original(buffer);

  BufferHandle moved(std::move(original));
  EXPECT_TRUE(moved);
  EXPECT_FALSE(original); // Moved from
}

TEST_F(BufferTest, BufferHandleMoveAssignment) {
  const size_t size1 = 100;
  const size_t size2 = 200;

  Buffer *buffer1 = createMemoryBlock(size1);
  Buffer *buffer2 = createMemoryBlock(size2);

  BufferHandle handle1(buffer1);
  BufferHandle handle2(buffer2);

  handle1 = std::move(handle2);

  EXPECT_TRUE(handle1);
  EXPECT_EQ(handle1->get_size(), size2);
  EXPECT_FALSE(handle2);
}

TEST_F(BufferTest, BufferHandleRelease) {
  const size_t size = 128;
  Buffer *buffer = createMemoryBlock(size);
  BufferHandle handle(buffer);

  Buffer *released = handle.relinquish();
  EXPECT_EQ(released, buffer);
  EXPECT_FALSE(handle);

  Buffer::destroy(released);
}

TEST_F(BufferTest, SharedBufferHandleConstruction) {
  const size_t size = 256;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);

  SharedBufferHandle handle(buffer);
  EXPECT_TRUE(handle);
  EXPECT_EQ(buffer->reference_count.load(), 1);
}

TEST_F(BufferTest, SharedBufferHandleCopyConstruction) {
  const size_t size = 128;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);

  SharedBufferHandle handle1(buffer);
  SharedBufferHandle handle2(handle1);

  EXPECT_TRUE(handle1);
  EXPECT_TRUE(handle2);
  EXPECT_EQ(buffer->reference_count.load(), 2);
}

class SharedBufferHandleTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SharedBufferHandleTest, SharedBufferHandleAssignment) {
  const size_t size = 64;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);

  SharedBufferHandle handle1(buffer);
  SharedBufferHandle handle2;

  handle2 = handle1;

  EXPECT_TRUE(handle1);
  EXPECT_TRUE(handle2);
  EXPECT_EQ(buffer->reference_count.load(), 2);
}

TEST_F(BufferTest, SharedBufferHandleRefCounting) {
  const size_t size = 512;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);

  {
    SharedBufferHandle handle1(buffer);
    EXPECT_EQ(buffer->reference_count.load(), 1);

    {
      SharedBufferHandle handle2(handle1);
      EXPECT_EQ(buffer->reference_count.load(), 2);

      {
        SharedBufferHandle handle3(handle2);
        EXPECT_EQ(buffer->reference_count.load(), 3);
      }

      EXPECT_EQ(buffer->reference_count.load(), 2);
    }

    EXPECT_EQ(buffer->reference_count.load(), 1);
  }

  EXPECT_EQ(buffer->reference_count.load(), 0);
}

TEST_F(BufferTest, MakeBufferFunction) {
  const size_t size = 1024;
  BufferHandle handle = createMemoryBlock(size);

  EXPECT_TRUE(handle);
  EXPECT_EQ(handle->get_size(), size);
}

TEST_F(BufferTest, BufferAlignment) {
  const size_t size = 1024;
  Buffer *buffer = createMemoryBlock(size);

  // Test that the buffer is properly aligned
  uintptr_t data_addr = reinterpret_cast<uintptr_t>(buffer->get_data());
  EXPECT_EQ(data_addr % alignof(std::max_align_t), 0);

  // Test that we can write to all bytes
  std::byte *data = buffer->get_data();
  for (size_t i = 0; i < size; ++i) {
    data[i] = std::byte(0xFF);
  }

  Buffer::destroy(buffer);
}

TEST_F(BufferTest, LargeBufferAllocation) {
  const size_t size = 1024 * 1024; // 1MB
  Buffer *buffer = createMemoryBlock(size);

  EXPECT_NE(buffer, nullptr);
  EXPECT_EQ(buffer->get_size(), size);

  // Test that we can access the data
  std::byte *data = buffer->get_data();
  EXPECT_NE(data, nullptr);

  // Write to first and last byte
  data[0] = std::byte(0xAA);
  data[size - 1] = std::byte(0xBB);

  EXPECT_EQ(data[0], std::byte(0xAA));
  EXPECT_EQ(data[size - 1], std::byte(0xBB));

  Buffer::destroy(buffer);
}

TEST_F(BufferTest, ZeroSizeBuffer) {
  Buffer *buffer = createMemoryBlock(0);

  EXPECT_NE(buffer, nullptr);
  EXPECT_EQ(buffer->get_size(), 0);

  Buffer::destroy(buffer);
}

TEST_F(BufferTest, BufferHandleOperatorBool) {
  const size_t size = 256;
  Buffer *buffer = createMemoryBlock(size);
  BufferHandle valid_handle(buffer);
  BufferHandle invalid_handle;

  EXPECT_TRUE(valid_handle);
  EXPECT_FALSE(invalid_handle);
}

TEST_F(BufferTest, SharedBufferHandleOperatorBool) {
  const size_t size = 128;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);
  SharedBufferHandle valid_handle(buffer);
  SharedBufferHandle invalid_handle;

  EXPECT_TRUE(valid_handle);
  EXPECT_FALSE(invalid_handle);
}

TEST_F(BufferTest, BufferHandleOperatorArrow) {
  const size_t size = 512;
  Buffer *buffer = createMemoryBlock(size);
  BufferHandle handle(buffer);

  EXPECT_EQ(handle->get_size(), size);
  EXPECT_EQ(handle->reference_count.load(), 0);
}

TEST_F(BufferTest, SharedBufferHandleOperatorArrow) {
  const size_t size = 256;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);
  SharedBufferHandle handle(buffer);

  EXPECT_EQ(handle->get_size(), size);
  EXPECT_EQ(handle->reference_count.load(), 1);
}

TEST_F(BufferTest, BufferHandleConversion) {
  const size_t size = 128;
  Buffer *buffer = createMemoryBlock(size);
  BufferHandle handle(buffer);

  Buffer *converted = handle;
  EXPECT_EQ(converted, buffer);
}

TEST_F(BufferTest, SharedBufferHandleConversion) {
  const size_t size = 64;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);
  SharedBufferHandle handle(buffer);

  Buffer *converted = handle;
  EXPECT_EQ(converted, buffer);
}

TEST_F(BufferTest, MultipleBufferHandles) {
  const size_t size1 = 100;
  const size_t size2 = 200;

  BufferHandle handle1 = createMemoryBlock(size1);
  BufferHandle handle2 = createMemoryBlock(size2);

  EXPECT_TRUE(handle1);
  EXPECT_TRUE(handle2);
  EXPECT_EQ(handle1->get_size(), size1);
  EXPECT_EQ(handle2->get_size(), size2);
}

TEST_F(BufferTest, SharedBufferHandleMultipleReferences) {
  const size_t size = 1024;
  Buffer *buffer = createMemoryBlock(size);
  buffer->reference_count.store(0, std::memory_order_relaxed);

  SharedBufferHandle handle1(buffer);
  SharedBufferHandle handle2(buffer);
  SharedBufferHandle handle3(buffer);

  EXPECT_EQ(buffer->reference_count.load(), 3);
}