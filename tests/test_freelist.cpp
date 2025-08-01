#include "freelist.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace turbokit;

class FreeListTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

struct TestObject {
  int value;
  void *next;

  TestObject(int v = 0) : value(v), next(nullptr) {}
};

TEST_F(FreeListTest, BasicPushPop) {
  const size_t max_thread_local = 10;

  // Push some objects
  for (int i = 0; i < 5; ++i) {
    auto *obj = new TestObject(i);
    FreeList<TestObject>::add_element(obj, max_thread_local);
  }

  // Pop them back
  for (int i = 0; i < 5; ++i) {
    auto *obj = FreeList<TestObject>::remove_element();
    EXPECT_NE(obj, nullptr);
    delete obj;
  }

  // Should be empty now
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, ThreadLocalStorage) {
  const size_t max_thread_local = 5;

  // Fill thread local storage
  for (size_t i = 0; i < max_thread_local; ++i) {
    auto *obj = new TestObject(static_cast<int>(i));
    FreeList<TestObject>::add_element(obj, max_thread_local);
  }

  // Pop all back
  for (size_t i = 0; i < max_thread_local; ++i) {
    auto *obj = FreeList<TestObject>::remove_element();
    EXPECT_NE(obj, nullptr);
    delete obj;
  }

  // Should be empty
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, GlobalStorage) {
  const size_t max_thread_local = 3;

  // Push more than thread local capacity
  for (int i = 0; i < 10; ++i) {
    auto *obj = new TestObject(i);
    FreeList<TestObject>::add_element(obj, max_thread_local);
  }

  // Pop all back
  for (int i = 0; i < 10; ++i) {
    auto *obj = FreeList<TestObject>::remove_element();
    EXPECT_NE(obj, nullptr);
    delete obj;
  }

  // Should be empty
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, EmptyPop) {
  // Try to pop from empty list
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, MultipleThreads) {
  const size_t max_thread_local = 5;
  const size_t num_threads = 4;
  const size_t objects_per_thread = 100;

  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([max_thread_local, objects_per_thread]() {
      // Each thread pushes and pops objects
      for (size_t j = 0; j < objects_per_thread; ++j) {
        auto *obj = new TestObject(static_cast<int>(j));
        FreeList<TestObject>::add_element(obj, max_thread_local);
      }

      for (size_t j = 0; j < objects_per_thread; ++j) {
        auto *obj = FreeList<TestObject>::remove_element();
        EXPECT_NE(obj, nullptr);
        delete obj;
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }
}

TEST_F(FreeListTest, ObjectReuse) {
  const size_t max_thread_local = 10;

  // Push an object
  auto *obj = new TestObject(42);
  FreeList<TestObject>::add_element(obj, max_thread_local);

  // Pop it back
  auto *popped = FreeList<TestObject>::remove_element();
  EXPECT_EQ(popped, obj);
  EXPECT_EQ(popped->value, 42);

  // Push it again
  FreeList<TestObject>::add_element(popped, max_thread_local);

  // Pop it again
  auto *popped2 = FreeList<TestObject>::remove_element();
  EXPECT_EQ(popped2, obj);
  EXPECT_EQ(popped2->value, 42);

  delete popped2;
}

TEST_F(FreeListTest, LargeObjectPool) {
  const size_t max_thread_local = 5;

  // Push many objects
  for (int i = 0; i < 50; ++i) {
    auto *obj = new TestObject(i);
    FreeList<TestObject>::add_element(obj, max_thread_local);
  }

  // Pop them back
  for (int i = 0; i < 50; ++i) {
    auto *obj = FreeList<TestObject>::remove_element();
    EXPECT_NE(obj, nullptr);
    delete obj;
  }

  // Should be empty
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, StressTest) {
  const size_t max_thread_local = 10;
  const size_t num_threads = 8;
  const size_t iterations = 1000;

  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([max_thread_local, iterations]() {
      for (size_t j = 0; j < iterations; ++j) {
        auto *obj = new TestObject(static_cast<int>(j));
        FreeList<TestObject>::add_element(obj, max_thread_local);
      }

      for (size_t j = 0; j < iterations; ++j) {
        auto *obj = FreeList<TestObject>::remove_element();
        EXPECT_NE(obj, nullptr);
        delete obj;
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }
}

TEST_F(FreeListTest, ZeroMaxThreadLocal) {
  const size_t max_thread_local = 0;

  // Push an object
  auto *obj = new TestObject(42);
  FreeList<TestObject>::add_element(obj, max_thread_local);

  // Pop it back
  auto *popped = FreeList<TestObject>::remove_element();
  EXPECT_EQ(popped, obj);
  delete popped;
}

TEST_F(FreeListTest, NullObject) {
  const size_t max_thread_local = 10;

  // Push null object
  FreeList<TestObject>::add_element(nullptr, max_thread_local);

  // Pop it back
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, MemoryLeakPrevention) {
  const size_t max_thread_local = 5;

  // Push some objects
  for (int i = 0; i < 3; ++i) {
    auto *obj = new TestObject(i);
    FreeList<TestObject>::add_element(obj, max_thread_local);
  }

  // Pop them back and delete
  for (int i = 0; i < 3; ++i) {
    auto *obj = FreeList<TestObject>::remove_element();
    EXPECT_NE(obj, nullptr);
    delete obj;
  }

  // Should be empty
  auto *obj = FreeList<TestObject>::remove_element();
  EXPECT_EQ(obj, nullptr);
}

TEST_F(FreeListTest, DifferentObjectTypes) {
  struct LargeObject {
    char data[1024];
    void *next;

    LargeObject() : next(nullptr) { std::fill(data, data + sizeof(data), 'A'); }
  };

  const size_t max_thread_local = 10;

  // Push large object
  auto *obj = new LargeObject();
  FreeList<LargeObject>::add_element(obj, max_thread_local);

  // Pop it back
  auto *popped = FreeList<LargeObject>::remove_element();
  EXPECT_EQ(popped, obj);
  delete popped;
}