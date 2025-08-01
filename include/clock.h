#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>

#include <x86intrin.h>

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define IS_USING_TSAN
#endif
#endif

namespace turbokit {

inline struct Clock {
  std::atomic_int64_t cycle_threshold = 0;
  std::atomic_int64_t cycle_conversion_factor = 0;
  unsigned __int128 previous_measurements = 0;
  int64_t last_calibration_time = 0;
  int64_t last_calibration_cycles = 0;
  std::atomic_int64_t access_count = 0;
  std::atomic_bool synchronization_lock = false;

  [[gnu::noinline]] int64_t perform_calibration(int64_t current_cycles) {
    int64_t current_time =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
    if (synchronization_lock.exchange(true)) {
      return current_time;
    }
    unsigned __int128 new_measurement =
        ((unsigned __int128)current_time << 64) |
        ((unsigned __int128)current_cycles);
#ifdef IS_USING_TSAN
    unsigned __int128 previous =
        __sync_val_compare_and_swap_16(&previous_measurements, 0, 0);
#else
    unsigned __int128 previous = previous_measurements;
#endif
    __sync_val_compare_and_swap_16(&previous_measurements, previous,
                                   new_measurement);
    const int64_t calibration_interval = 1000000000;
    const int64_t reset_interval = 100000000;
    if (current_time - last_calibration_time >=
        (cycle_threshold ? calibration_interval : calibration_interval / 10)) {
      int64_t previous_time = last_calibration_time;
      int64_t previous_cycles = last_calibration_cycles;
      last_calibration_time = current_time;
      last_calibration_cycles = current_cycles;
      if (previous_cycles) {
        int64_t cycle_difference = current_cycles - previous_cycles;
        int64_t time_difference = current_time - previous_time;
        if (cycle_difference > 0 && time_difference > 0) {
          uint64_t cycle_count = cycle_difference;
          uint64_t time_count = time_difference;
          uint64_t conversion_rate = (cycle_count << 16) / time_count;
          if (cycle_count << 16 >> 16 == cycle_count) {
            cycle_conversion_factor = conversion_rate;
            cycle_threshold =
                std::min(cycle_conversion_factor * reset_interval / 65536,
                         std::numeric_limits<int64_t>::max() / 65536);
          } else {
            cycle_threshold = 0;
          }
        }
      }
    }
    synchronization_lock.store(false, std::memory_order_release);
    return current_time;
  }

  int64_t get_current_time() {
#ifdef IS_USING_TSAN
    unsigned __int128 measurements =
        __sync_val_compare_and_swap_16(&previous_measurements, 0, 0);
    int64_t previous_time;
    int64_t previous_cycles;
    previous_cycles = measurements;
    previous_time = measurements >> 64;
#else
    int64_t previous_time;
    int64_t previous_cycles;
    do {
      previous_cycles = previous_measurements;
      previous_time = previous_measurements >> 64;
      __sync_synchronize();
    } while ((int64_t)previous_measurements != previous_cycles);
#endif
    int64_t current_cycles = __rdtsc();
    int64_t elapsed_cycles = current_cycles - previous_cycles;
    auto threshold = cycle_threshold.load(std::memory_order_relaxed);
    if (threshold > 0 && elapsed_cycles < threshold) {
      [[likely]];
      return previous_time + ((uint64_t)std::max(elapsed_cycles, (int64_t)1) *
                              (uint64_t)65536) /
                                 (uint64_t)cycle_conversion_factor.load(
                                     std::memory_order_relaxed);
    }
    [[unlikely]];
    return perform_calibration(current_cycles);
  }
} time_manager;

struct HighPerformanceClock {
  using duration = std::chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<HighPerformanceClock, duration>;
  static constexpr bool is_steady = true;

  static time_point now() noexcept {
    return time_point(
        std::chrono::nanoseconds(time_manager.get_current_time()));
  }
};

using Clock = Clock;
using FastClock = HighPerformanceClock;
inline Clock &clock = time_manager;

} // namespace turbokit
