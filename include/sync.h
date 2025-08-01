#pragma once

#include "clock.h"

#include <atomic>
#include <climits>
#include <cstdio>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include <linux/futex.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <x86intrin.h>

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define IS_USING_TSAN
#endif
#endif

#ifdef IS_USING_TSAN
#include <sanitizer/tsan_interface.h>
#else

#define __tsan_mutex_pre_lock(...)
#define __tsan_mutex_post_lock(...)
#define __tsan_mutex_pre_unlock(...)
#define __tsan_mutex_post_unlock(...)

#endif

namespace turbokit {

inline void wakeAllThreads(std::atomic_uint32_t *synchronization_object) {
  syscall(SYS_futex, (void *)synchronization_object, FUTEX_WAKE, INT_MAX,
          nullptr);
}

inline void waitForCondition(std::atomic_uint32_t *synchronization_object,
                             uint32_t expected_value,
                             std::chrono::nanoseconds timeout_duration) {
  auto nanoseconds = timeout_duration;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nanoseconds);
  timespec time_spec;
  time_spec.tv_sec = seconds.count();
  time_spec.tv_nsec = (nanoseconds - seconds).count();
  syscall(SYS_futex, (void *)synchronization_object, FUTEX_WAIT, expected_value,
          (void *)&time_spec);
}

inline void waitUntilValueReached(std::atomic_uint32_t *synchronization_object,
                                  uint32_t target_value) {
  uint32_t current_value = synchronization_object->load();
  while (current_value < target_value) {
    waitForCondition(synchronization_object, current_value,
                     std::chrono::seconds(1));
    current_value = synchronization_object->load();
  }
}

inline void
waitUntilValueReachedWithDebug(const char *function_name, int line_number,
                               std::atomic_uint32_t *synchronization_object,
                               uint32_t target_value) {
  auto start_time = std::chrono::steady_clock::now();
  uint32_t current_value = synchronization_object->load();
  while (current_value < target_value) {
    waitForCondition(synchronization_object, current_value,
                     std::chrono::milliseconds(1));
    current_value = synchronization_object->load();
    if (std::chrono::steady_clock::now() - start_time >=
        std::chrono::milliseconds(500)) {
      printf("deadlocked on futex wait at %s:%d\n", function_name, line_number);
    }
  }
}

// #define waitUntilValueReached(a, b) waitUntilValueReachedWithDebug(__FILE__,
// __LINE__, a, b)

#if 0
using SpinMutex = std::mutex;
#elif 0
inline std::atomic_int mutexThreadIdCounter = 0;
inline thread_local int mutexThreadId = mutexThreadIdCounter++;
class SpinMutex {
  int magic = 0x42;
  std::atomic<bool> locked_{false};
  std::atomic<int *> owner = nullptr;
  std::atomic<void *> ownerAddress = nullptr;

public:
  void lock() {
    if (owner == &mutexThreadId) {
      printf("recursive lock\n");
      std::abort();
    }
    if (magic != 0x42) {
      printf("BAD MUTEX MAGIC\n");
      std::abort();
    }
    auto start = Clock::now();
    do {
      while (locked_.load(std::memory_order_acquire)) {
        _mm_pause();
        if (magic != 0x42) {
          printf("BAD MUTEX MAGIC\n");
          std::abort();
        }
        if (Clock::now() - start >= std::chrono::milliseconds(100)) {
          int *p = owner.load();
          printf("deadlock detected in thread %d! held by thread %d (my return "
                 "address %p, owner return address %p, "
                 "&mutexThreadIdCounter is %p)\n",
                 mutexThreadId, p ? *p : -1, __builtin_return_address(0),
                 ownerAddress.load(), (void *)&mutexThreadIdCounter);
          start = Clock::now();
        }
      }
    } while (locked_.exchange(true, std::memory_order_acq_rel));
    owner = &mutexThreadId;
    ownerAddress = __builtin_return_address(0);
  }
  void unlock() {
    if (magic != 0x42) {
      printf("BAD MUTEX MAGIC\n");
      std::abort();
    }
    owner = nullptr;
    locked_.store(false);
  }
  bool try_lock() {
    if (owner == &mutexThreadId) {
      printf("recursive try_lock\n");
      std::abort();
    }
    if (locked_.load(std::memory_order_acquire)) {
      return false;
    }
    bool r = !locked_.exchange(true, std::memory_order_acq_rel);
    if (r) {
      owner = &mutexThreadId;
    }
    return r;
  }
};
#else
class SpinMutex {
  std::atomic<bool> is_locked = false;

public:
  void lock() {
    __tsan_mutex_pre_lock(this, 0);
    do {
      while (is_locked.load(std::memory_order_relaxed)) {
        _mm_pause();
      }
    } while (is_locked.exchange(true, std::memory_order_acq_rel));
    __tsan_mutex_post_lock(this, 0, 0);
  }
  void unlock() {
    __tsan_mutex_pre_unlock(this, 0);
    is_locked.store(false, std::memory_order_release);
    __tsan_mutex_post_unlock(this, 0);
  }
  bool try_lock() {
    __tsan_mutex_pre_lock(this, __tsan_mutex_try_lock);
    if (is_locked.load(std::memory_order_relaxed)) {
      __tsan_mutex_post_lock(
          this, __tsan_mutex_try_lock | __tsan_mutex_try_lock_failed, 0);
      return false;
    }
    if (!is_locked.exchange(true, std::memory_order_acq_rel)) {
      __tsan_mutex_post_lock(this, __tsan_mutex_try_lock, 0);
      return true;
    } else {
      __tsan_mutex_post_lock(
          this, __tsan_mutex_try_lock | __tsan_mutex_try_lock_failed, 0);
      return false;
    }
  }
};
#endif

#if 0
using SharedSpinMutex = std::shared_mutex;
#else
class SharedSpinMutex {
  std::atomic_bool is_locked = false;
  std::atomic_int shared_count = 0;

public:
  void lock() {
    __tsan_mutex_pre_lock(this, 0);
    do {
      while (is_locked.load(std::memory_order_relaxed)) {
        _mm_pause();
      }
    } while (is_locked.exchange(true));
    while (shared_count.load()) {
      _mm_pause();
    }
    __tsan_mutex_post_lock(this, 0, 0);
  }
  void unlock() {
    __tsan_mutex_pre_unlock(this, 0);
    is_locked.store(false, std::memory_order_release);
    __tsan_mutex_post_unlock(this, 0);
  }
  bool try_lock() {
    __tsan_mutex_pre_lock(this, __tsan_mutex_try_lock);
    if (is_locked.load(std::memory_order_relaxed)) {
      __tsan_mutex_post_lock(
          this, __tsan_mutex_try_lock | __tsan_mutex_try_lock_failed, 0);
      return false;
    }
    if (shared_count.load(std::memory_order_relaxed) == 0 &&
        !is_locked.exchange(true, std::memory_order_acq_rel)) {
      if (shared_count.load(std::memory_order_relaxed) == 0) {
        __tsan_mutex_post_lock(this, __tsan_mutex_try_lock, 0);
        return true;
      } else {
        is_locked.store(false, std::memory_order_release);
      }
    }
    __tsan_mutex_post_lock(
        this, __tsan_mutex_try_lock | __tsan_mutex_try_lock_failed, 0);
    return false;
  }
  void lock_shared() {
    __tsan_mutex_pre_lock(this, __tsan_mutex_read_lock);
    while (true) {
      while (is_locked.load(std::memory_order_relaxed)) {
        _mm_pause();
      }
      shared_count.fetch_add(1);
      if (is_locked.load()) {
        shared_count.fetch_sub(1, std::memory_order_acq_rel);
      } else {
        break;
      }
    }
    __tsan_mutex_post_lock(this, __tsan_mutex_read_lock, 0);
  }
  void unlock_shared() {
    __tsan_mutex_pre_unlock(this, __tsan_mutex_read_lock);
    shared_count.fetch_sub(1, std::memory_order_release);
    __tsan_mutex_post_unlock(this, __tsan_mutex_read_lock);
  }
  bool try_lock_shared() {
    __tsan_mutex_pre_lock(this, __tsan_mutex_try_lock | __tsan_mutex_read_lock);
    if (is_locked.load(std::memory_order_relaxed)) {
      return false;
    }
    shared_count.fetch_add(1);
    if (is_locked.load()) {
      shared_count.fetch_sub(1, std::memory_order_acq_rel);
      __tsan_mutex_post_lock(this,
                             __tsan_mutex_try_lock |
                                 __tsan_mutex_try_lock_failed |
                                 __tsan_mutex_read_lock,
                             0);
      return false;
    }
    __tsan_mutex_post_lock(this, __tsan_mutex_try_lock | __tsan_mutex_read_lock,
                           0);
    return true;
  }
};
#endif

class ThreadSynchronizer {
  sem_t semaphore;

public:
  ThreadSynchronizer() noexcept { sem_init(&semaphore, 0, 0); }
  ~ThreadSynchronizer() { sem_destroy(&semaphore); }
  void signal() noexcept { sem_post(&semaphore); }
  void wait() noexcept {
    while (sem_wait(&semaphore)) {
      if (errno != EINTR) {
        printf("sem_wait returned errno %d", (int)errno);
        std::abort();
      }
    }
  }
  template <typename Rep, typename Period>
  void wait_for(const std::chrono::duration<Rep, Period> &duration) noexcept {
    struct timespec time_spec;
    auto absolute_time =
        std::chrono::system_clock::now().time_since_epoch() + duration;
    auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(absolute_time);
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(nanoseconds);
    time_spec.tv_sec = seconds.count();
    time_spec.tv_nsec = (nanoseconds - seconds).count();
    while (sem_timedwait(&semaphore, &time_spec)) {
      if (errno == ETIMEDOUT) {
        break;
      }
      if (errno != EINTR) {
        printf("sem_timedwait returned errno %d", (int)errno);
        std::abort();
      }
    }
  }
  template <typename Clock, typename Duration>
  void wait_until(
      const std::chrono::time_point<Clock, Duration> &time_point) noexcept {
    wait_for(time_point - Clock::now());
  }

  ThreadSynchronizer(const ThreadSynchronizer &) = delete;
  ThreadSynchronizer(const ThreadSynchronizer &&) = delete;
  ThreadSynchronizer &operator=(const ThreadSynchronizer &) = delete;
  ThreadSynchronizer &operator=(const ThreadSynchronizer &&) = delete;
};

using Semaphore = ThreadSynchronizer;

} // namespace turbokit
