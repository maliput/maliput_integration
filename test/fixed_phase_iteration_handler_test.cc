#include "integration/fixed_phase_iteration_handler.h"

#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include <maliput/api/intersection.h>
#include <maliput/api/intersection_book.h>
#include <maliput/api/road_network.h>
#include <maliput/api/rules/phase.h>
#include <maliput/common/assertion_error.h>
#include <maliput/common/filesystem.h>

#include "integration/create_timer.h"
#include "integration/tools.h"

namespace maliput {
namespace integration {
namespace {

constexpr char MALIPUT_MALIDRIVE_RESOURCE_VAR[] = "MALIPUT_MALIDRIVE_RESOURCE_ROOT";

// Uses maliput_malidrive's SingleRoadPedestrianCrosswalk phase rings to evaluate the FixedPhaseIterationHandler
// implementation.
class FixedPhaseIterationHandlerTest : public ::testing::Test {
 public:
  static constexpr char kYamlFileName[] = "/resources/odr/SingleRoadPedestrianCrosswalk.yaml";
  static constexpr char kXodrFileName[] = "/resources/odr/SingleRoadPedestrianCrosswalk.xodr";
  const std::string kXodrFilePath{maliput::common::Filesystem::get_env_path(MALIPUT_MALIDRIVE_RESOURCE_VAR) +
                                  kXodrFileName};
  const std::string kYamlFilePath{maliput::common::Filesystem::get_env_path(MALIPUT_MALIDRIVE_RESOURCE_VAR) +
                                  kYamlFileName};
  const double kPhaseDuration{0.5};

  void SetUp() override {
    MalidriveBuildProperties properties{};
    properties.xodr_file_path = kXodrFilePath;
    properties.linear_tolerance = 5e-2;
    properties.rule_registry_file = kYamlFilePath;
    properties.road_rule_book_file = kYamlFilePath;
    properties.traffic_light_book_file = kYamlFilePath;
    properties.phase_ring_book_file = kYamlFilePath;
    properties.intersection_book_file = kYamlFilePath;
    rn_ = CreateMalidriveRoadNetwork(properties);
    ASSERT_NE(rn_, nullptr);
    timer_ = CreateTimer(TimerType::kChronoTimer);
    ASSERT_NE(timer_, nullptr);
  }

  std::unique_ptr<api::RoadNetwork> rn_;
  std::unique_ptr<Timer> timer_;
};

TEST_F(FixedPhaseIterationHandlerTest, Constructor) {
  EXPECT_THROW(FixedPhaseIterationHandler(timer_.get(), rn_.get(), -5.), maliput::common::assertion_error);
  EXPECT_NO_THROW(FixedPhaseIterationHandler(timer_.get(), rn_.get(), kPhaseDuration));
}

TEST_F(FixedPhaseIterationHandlerTest, VerifyPhasesBeingIterated) {
  const maliput::api::rules::Phase::Id kAllGoPhase{"AllGoPhase"};
  const maliput::api::rules::Phase::Id kAllStopPhase{"AllStopPhase"};

  // Obtains the intersection to be used for the analysis.
  maliput::api::IntersectionBook* intersection_book = rn_->intersection_book();
  ASSERT_NE(intersection_book, nullptr);
  const maliput::api::Intersection* intersection =
      intersection_book->GetIntersection(maliput::api::Intersection::Id("PedestrianCrosswalkIntersection"));
  ASSERT_NE(intersection, nullptr);

  // According to the IntersectionBook yaml file the initial phase is: AllGoPhase.
  EXPECT_EQ(kAllGoPhase, intersection->Phase()->state);

  timer_->Reset();
  FixedPhaseIterationHandler dut{timer_.get(), rn_.get(), kPhaseDuration};
  // Wait for the timer to go over the phase duration time.
  std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(kPhaseDuration * 1.1 * 1000.)));
  // Call the update method to iterate the phases given that we already waited the kPhaseDuration time.
  dut.Update();
  EXPECT_EQ(kAllStopPhase, intersection->Phase()->state);
}

}  // namespace
}  // namespace integration
}  // namespace maliput
