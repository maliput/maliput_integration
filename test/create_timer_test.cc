#include "integration/create_timer.h"

#include <chrono>
#include <memory>

#include <gtest/gtest.h>
#include <maliput/test_utilities/mock.h>

#include "integration/chrono_timer.h"

namespace maliput {
namespace integration {
namespace {

GTEST_TEST(CreateTimerTest, CreateTimer) {
  std::unique_ptr<Timer> timer = CreateTimer(TimerType::kChronoTimer);
  EXPECT_NE(timer, nullptr);
  EXPECT_NE(dynamic_cast<ChronoTimer*>(timer.get()), nullptr);
}

}  // namespace
}  // namespace integration
}  // namespace maliput
