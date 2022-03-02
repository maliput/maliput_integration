#include "integration/dynamic_environment_handler.h"

#include <memory>

#include <gtest/gtest.h>
#include <maliput/common/assertion_error.h>
#include <maliput/test_utilities/mock.h>

#include "integration/create_timer.h"

namespace maliput {
namespace integration {
namespace {

class MockDynamicEnvironmentHandler : public DynamicEnvironmentHandler {
 public:
  MockDynamicEnvironmentHandler(const Timer* timer, api::RoadNetwork* road_network)
      : DynamicEnvironmentHandler(timer, road_network) {}

  void Update() override { call_update_ = true; }
  bool call_update_{false};
};

class DynamicEnvironmentHandlerTest : public ::testing::Test {
 public:
  void SetUp() override {
    ASSERT_NE(rn_, nullptr);
    ASSERT_NE(timer_, nullptr);
  }

  std::unique_ptr<Timer> timer_ = CreateTimer(TimerType::kChronoTimer);
  std::unique_ptr<maliput::api::RoadNetwork> rn_ = maliput::api::test::CreateRoadNetwork();
};

TEST_F(DynamicEnvironmentHandlerTest, Constructor) {
  EXPECT_THROW(MockDynamicEnvironmentHandler(timer_.get(), nullptr), maliput::common::assertion_error);
  EXPECT_THROW(MockDynamicEnvironmentHandler(nullptr, rn_.get()), maliput::common::assertion_error);
  EXPECT_NO_THROW(MockDynamicEnvironmentHandler(timer_.get(), rn_.get()));
}

TEST_F(DynamicEnvironmentHandlerTest, API) {
  MockDynamicEnvironmentHandler mock_deh{timer_.get(), rn_.get()};
  DynamicEnvironmentHandler* dut = dynamic_cast<DynamicEnvironmentHandler*>(&mock_deh);
  // Update
  dut->Update();
  EXPECT_TRUE(mock_deh.call_update_);
}

}  // namespace
}  // namespace integration
}  // namespace maliput
