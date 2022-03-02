#include "integration/create_dynamic_environment_handler.h"

#include <chrono>
#include <memory>

#include <gtest/gtest.h>
#include <maliput/test_utilities/mock.h>

#include "integration/create_timer.h"
#include "integration/fixed_phase_iteration_handler.h"

namespace maliput {
namespace integration {
namespace {

GTEST_TEST(CreateDynamicEnvironmentHandlerTest, CreateDynamicEnvironmentHandler) {
  auto rn = maliput::api::test::CreateRoadNetwork();
  ASSERT_NE(rn, nullptr);
  auto timer = CreateTimer(TimerType::kChronoTimer);
  ASSERT_NE(timer, nullptr);
  const double kPhaseDuration{1};
  std::unique_ptr<DynamicEnvironmentHandler> deh = CreateDynamicEnvironmentHandler(
      DynamicEnvironmentHandlerType::kFixedPhaseIterationHandler, timer.get(), rn.get(), kPhaseDuration);

  EXPECT_NE(deh, nullptr);
  EXPECT_NE(dynamic_cast<FixedPhaseIterationHandler*>(deh.get()), nullptr);
}

}  // namespace
}  // namespace integration
}  // namespace maliput
