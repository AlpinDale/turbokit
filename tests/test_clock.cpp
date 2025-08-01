#include "clock.h"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace turbokit;

class ClockTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ClockTest, ClockInitialization) {
  // Test that clock is properly initialized
  EXPECT_NE(&time_manager, nullptr);
}

TEST_F(ClockTest, ClockNowReturnsValue) {
  auto time1 = time_manager.get_current_time();
  auto time2 = time_manager.get_current_time();

  EXPECT_GT(time2, time1); // Time should advance
  EXPECT_GT(time1, 0);     // Should return positive values
}

TEST_F(ClockTest, ClockMonotonicity) {
  std::vector<int64_t> times;
  const size_t count = 100;

  for (size_t i = 0; i < count; ++i) {
    times.push_back(time_manager.get_current_time());
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  // Verify monotonicity
  for (size_t i = 1; i < times.size(); ++i) {
    EXPECT_GE(times[i], times[i - 1]);
  }
}

TEST_F(ClockTest, ClockPrecision) {
  auto start = time_manager.get_current_time();
  std::this_thread::sleep_for(std::chrono::nanoseconds(1000)); // 1 microsecond
  auto end = time_manager.get_current_time();

  auto duration = end - start;
  EXPECT_GT(duration, 0); // Should detect the sleep
}

TEST_F(ClockTest, FastClockCompatibility) {
  auto fast_time1 = HighPerformanceClock::now();
  auto fast_time2 = HighPerformanceClock::now();

  EXPECT_GT(fast_time2, fast_time1);

  // Test that both clocks return reasonable values
  auto regular_time = time_manager.get_current_time();
  auto fast_time = HighPerformanceClock::now();

  // Both should return positive values
  EXPECT_GT(regular_time, 0);
  EXPECT_GT(fast_time.time_since_epoch().count(), 0);
}

TEST_F(ClockTest, ClockCalibration) {
  // Test that clock calibration works
  auto time1 = time_manager.get_current_time();

  // Force some calibration by doing many calls
  for (int i = 0; i < 1000; ++i) {
    time_manager.get_current_time();
  }

  auto time2 = time_manager.get_current_time();
  EXPECT_GT(time2, time1);
}

TEST_F(ClockTest, ClockPerformance) {
  const size_t iterations = 10000;
  auto start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < iterations; ++i) {
    time_manager.get_current_time();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Clock calls should be very fast (less than 1 microsecond per call on
  // average)
  double avg_time_per_call = static_cast<double>(duration.count()) / iterations;
  EXPECT_LT(avg_time_per_call, 1.0);
}

TEST_F(ClockTest, ClockConsistency) {
  auto time1 = time_manager.get_current_time();
  auto time2 = time_manager.get_current_time();
  auto time3 = time_manager.get_current_time();

  // All times should be monotonically increasing
  EXPECT_GE(time2, time1);
  EXPECT_GE(time3, time2);
}

TEST_F(ClockTest, FastClockProperties) {
  auto start = HighPerformanceClock::now();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  auto end = HighPerformanceClock::now();

  auto duration = end - start;
  EXPECT_GT(duration.count(), 0);
}

TEST_F(ClockTest, ClockThreadSafety) {
  const size_t num_threads = 4;
  const size_t calls_per_thread = 1000;

  std::vector<std::thread> threads;
  std::vector<std::vector<int64_t>> results(num_threads);

  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&, i]() {
      for (size_t j = 0; j < calls_per_thread; ++j) {
        results[i].push_back(time_manager.get_current_time());
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  // Verify that all threads got monotonically increasing times
  for (const auto &thread_results : results) {
    for (size_t i = 1; i < thread_results.size(); ++i) {
      EXPECT_GE(thread_results[i], thread_results[i - 1]);
    }
  }
}

TEST_F(ClockTest, ClockResolution) {
  std::vector<int64_t> deltas;
  const size_t samples = 1000;

  for (size_t i = 0; i < samples; ++i) {
    auto start = time_manager.get_current_time();
    auto end = time_manager.get_current_time();
    deltas.push_back(end - start);
  }

  // Most calls should be very fast (close to zero delta)
  size_t fast_calls = 0;
  for (auto delta : deltas) {
    if (delta < 1000) { // Less than 1 microsecond
      ++fast_calls;
    }
  }

  EXPECT_GT(fast_calls, samples * 0.8); // At least 80% should be fast
}

TEST_F(ClockTest, ClockOverflow) {
  // Test that clock handles large time values correctly
  auto start = time_manager.get_current_time();

  // Simulate a long-running application
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  auto end = time_manager.get_current_time();
  auto duration = end - start;

  EXPECT_GT(duration, 0);
  EXPECT_LT(duration, 1000000000); // Should be less than 1 second
}

TEST_F(ClockTest, FastClockDuration) {
  auto start = HighPerformanceClock::now();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  auto end = HighPerformanceClock::now();

  auto duration = end - start;
  auto duration_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

  EXPECT_GT(duration_ns.count(), 0);
  EXPECT_LT(duration_ns.count(), 1000000000); // Less than 1 second
}

TEST_F(ClockTest, ClockComparison) {
  auto time1 = time_manager.get_current_time();
  auto time2 = time_manager.get_current_time();
  auto time3 = time_manager.get_current_time();

  EXPECT_LT(time1, time2);
  EXPECT_LT(time2, time3);
  EXPECT_GT(time3, time1);
}

TEST_F(ClockTest, FastClockComparison) {
  auto time1 = HighPerformanceClock::now();
  auto time2 = HighPerformanceClock::now();
  auto time3 = HighPerformanceClock::now();

  EXPECT_LT(time1, time2);
  EXPECT_LT(time2, time3);
  EXPECT_GT(time3, time1);
}

TEST_F(ClockTest, ClockArithmetic) {
  auto time1 = time_manager.get_current_time();
  auto time2 = time_manager.get_current_time();

  auto diff = time2 - time1;
  EXPECT_GE(diff, 0);

  auto sum = time1 + diff;
  EXPECT_EQ(sum, time2);
}

TEST_F(ClockTest, FastClockArithmetic) {
  auto time1 = HighPerformanceClock::now();
  auto time2 = HighPerformanceClock::now();

  auto diff = time2 - time1;
  EXPECT_GE(diff.count(), 0);

  auto sum = time1 + diff;
  EXPECT_EQ(sum, time2);
}