#include "integration/timer.h"

#include <gtest/gtest.h>

namespace maliput {
namespace integration {
namespace {

class MockTimer : public Timer {
 public:
  static constexpr double kElapsedTime{123};

  MockTimer() : Timer() {}

  bool call_reset_{false};

 private:
  void DoReset() override { call_reset_ = true; }
  double DoElapsed() const override { return 123; }
};

GTEST_TEST(TimerTest, API) {
  MockTimer mock_timer{};
  Timer* dut = dynamic_cast<Timer*>(&mock_timer);

  // Reset
  dut->Reset();
  EXPECT_TRUE(mock_timer.call_reset_);

  // Elapsed
  EXPECT_EQ(MockTimer::kElapsedTime, dut->Elapsed());
}

}  // namespace
}  // namespace integration
}  // namespace maliput
