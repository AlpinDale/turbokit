#include "sync.h"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace turbokit;

class SyncTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SyncTest, SpinMutexBasic) {
  SpinMutex mutex;

  // Test basic lock/unlock
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();

  // Test that we can lock again
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexLock) {
  SpinMutex mutex;

  mutex.lock();
  EXPECT_FALSE(mutex.try_lock()); // Should fail to lock when already locked
  mutex.unlock();

  EXPECT_TRUE(mutex.try_lock()); // Should succeed after unlock
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexTryLock) {
  SpinMutex mutex;

  // First try_lock should succeed
  EXPECT_TRUE(mutex.try_lock());

  // Second try_lock should fail
  EXPECT_FALSE(mutex.try_lock());

  mutex.unlock();

  // After unlock, try_lock should succeed again
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexThreadSafety) {
  SpinMutex mutex;
  std::atomic<int> counter{0};
  const size_t num_threads = 4;
  const size_t iterations_per_thread = 1000;

  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&mutex, &counter, iterations_per_thread]() {
      for (size_t j = 0; j < iterations_per_thread; ++j) {
        std::lock_guard<SpinMutex> lock(mutex);
        ++counter;
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter.load(), num_threads * iterations_per_thread);
}

TEST_F(SyncTest, SpinMutexRAII) {
  SpinMutex mutex;

  {
    std::lock_guard<SpinMutex> lock(mutex);
    EXPECT_FALSE(mutex.try_lock()); // Should be locked
  } // lock goes out of scope, mutex should be unlocked

  EXPECT_TRUE(mutex.try_lock()); // Should be unlocked
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexUniqueLock) {
  SpinMutex mutex;

  {
    std::unique_lock<SpinMutex> lock(mutex);
    EXPECT_FALSE(mutex.try_lock()); // Should be locked
  } // lock goes out of scope, mutex should be unlocked

  EXPECT_TRUE(mutex.try_lock()); // Should be unlocked
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexDeferLock) {
  SpinMutex mutex;

  std::unique_lock<SpinMutex> lock(mutex, std::defer_lock);
  EXPECT_TRUE(mutex.try_lock()); // Should be unlocked initially

  mutex.unlock();
  lock.lock();                    // Now lock it
  EXPECT_FALSE(mutex.try_lock()); // Should be locked
}

TEST_F(SyncTest, SpinMutexAdoptLock) {
  SpinMutex mutex;
  mutex.lock();

  std::unique_lock<SpinMutex> lock(mutex, std::adopt_lock);
  EXPECT_FALSE(mutex.try_lock()); // Should be locked

  // lock will unlock when it goes out of scope
}

TEST_F(SyncTest, SpinMutexTryLockTimeout) {
  SpinMutex mutex;
  mutex.lock();

  // Try to lock with timeout - should fail
  auto start = std::chrono::steady_clock::now();
  bool locked = mutex.try_lock();
  auto end = std::chrono::steady_clock::now();

  EXPECT_FALSE(locked);

  // Should fail quickly (not wait)
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  EXPECT_LT(duration.count(), 1000); // Less than 1ms

  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexMultipleLocks) {
  SpinMutex mutex;

  // Test multiple rapid lock/unlock cycles
  for (int i = 0; i < 1000; ++i) {
    mutex.lock();
    mutex.unlock();
  }

  // Should still work after many cycles
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexConcurrentAccess) {
  SpinMutex mutex;
  std::atomic<int> shared_counter{0};
  const size_t num_threads = 8;
  const size_t iterations = 1000;

  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&mutex, &shared_counter, iterations]() {
      for (size_t j = 0; j < iterations; ++j) {
        std::lock_guard<SpinMutex> lock(mutex);
        int old_val = shared_counter.load();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        shared_counter.store(old_val + 1);
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(shared_counter.load(), num_threads * iterations);
}

TEST_F(SyncTest, SpinMutexDestruction) {
  // Test that mutex can be destroyed safely
  {
    SpinMutex mutex;
    mutex.lock();
    mutex.unlock();
  } // mutex destroyed here
}

TEST_F(SyncTest, SpinMutexCopyConstruction) {
  // SpinMutex should not be copyable
  static_assert(!std::is_copy_constructible_v<SpinMutex>);
  static_assert(!std::is_copy_assignable_v<SpinMutex>);
}

TEST_F(SyncTest, SpinMutexMoveConstruction) {
  // SpinMutex should not be movable
  static_assert(!std::is_move_constructible_v<SpinMutex>);
  static_assert(!std::is_move_assignable_v<SpinMutex>);
}

TEST_F(SyncTest, SpinMutexDefaultConstruction) {
  SpinMutex mutex;
  // Should be unlocked by default
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();
}

TEST_F(SyncTest, SpinMutexStressTest) {
  SpinMutex mutex;
  std::atomic<int> counter{0};
  const size_t num_threads = 16;
  const size_t iterations = 10000;

  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&mutex, &counter, iterations]() {
      for (size_t j = 0; j < iterations; ++j) {
        std::lock_guard<SpinMutex> lock(mutex);
        ++counter;
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter.load(), num_threads * iterations);
}