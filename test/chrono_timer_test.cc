#include "integration/chrono_timer.h"

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

namespace maliput {
namespace integration {
namespace {

GTEST_TEST(ChronoTimerTest, ChronoTimer) {
  const double kFirstSleep{0.250};  // seconds

  // Initialization.
  ChronoTimer dut{};
  EXPECT_LT(dut.Elapsed(), kFirstSleep);

  // Elapsed.
  std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(kFirstSleep * 1000)));
  EXPECT_GE(dut.Elapsed(), kFirstSleep);

  // Reset
  dut.Reset();
  EXPECT_LT(dut.Elapsed(), kFirstSleep);
}

}  // namespace
}  // namespace integration
}  // namespace maliput
