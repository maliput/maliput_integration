#include <gtest/gtest.h>

#include "integration/tools.h"

#include "maliput/common/filesystem.h"

#include "maliput_dragway/road_geometry.h"

#include "maliput_multilane/builder.h"
#include "maliput_multilane/loader.h"
#include "maliput_multilane/road_geometry.h"

namespace maliput {
namespace integration {
namespace {

constexpr char MULTILANE_RESOURCE_VAR[] = "MULTILANE_RESOURCE_ROOT";
constexpr char kFileName[] = "/2x2_intersection.yaml";
constexpr int kNumLanes{2};
constexpr double kLength{10.};
constexpr double kLaneWidth{3.7};
constexpr double kShoulderWidth{3.};
constexpr double kMaximumHeight{5.2};

GTEST_TEST(CreateRoadGeometryFrom, MultilaneRoadGeometry) {
  const std::string kFilePath{maliput::common::Filesystem::get_env_path(MULTILANE_RESOURCE_VAR) + kFileName};
  std::unique_ptr<const api::RoadGeometry> dut = CreateRoadGeometryFrom(kFilePath, std::nullopt);
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dynamic_cast<const multilane::RoadGeometry*>(dut.get()));
}

GTEST_TEST(CreateRoadGeometryFrom, DragwayRoadGeometry) {
  std::unique_ptr<const api::RoadGeometry> dut = CreateRoadGeometryFrom(
      std::nullopt, DragwayBuildProperties{kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dynamic_cast<const dragway::RoadGeometry*>(dut.get()));
}

GTEST_TEST(CreateRoadGeometryFrom, NoneImplementationSelected) {
  std::unique_ptr<const api::RoadGeometry> dut = CreateRoadGeometryFrom(std::nullopt, std::nullopt);
  EXPECT_EQ(nullptr, dut);
}

GTEST_TEST(CreateRoadGeometryFrom, BothImplementationSelected) {
  const std::string kFilePath{maliput::common::Filesystem::get_env_path(MULTILANE_RESOURCE_VAR) + kFileName};
  std::unique_ptr<const api::RoadGeometry> dut = CreateRoadGeometryFrom(
      kFilePath, DragwayBuildProperties{kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight});
  EXPECT_EQ(nullptr, dut);
}

}  // namespace
}  // namespace integration
}  // namespace maliput
