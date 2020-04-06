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

  std::optional<std::unique_ptr<const api::RoadGeometry>> dut =
      CreateRoadGeometryFrom(kFilePath, kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight);
  EXPECT_TRUE(dut.has_value());
  EXPECT_NE(nullptr,
            dynamic_cast<multilane::RoadGeometry*>(const_cast<maliput::api::RoadGeometry*>(dut.value().get())));
  EXPECT_EQ(nullptr, dynamic_cast<dragway::RoadGeometry*>(const_cast<maliput::api::RoadGeometry*>(dut.value().get())));
}

GTEST_TEST(CreateRoadGeometryFrom, DragwayRoadGeometry) {
  const std::string kFilePath{""};

  std::optional<std::unique_ptr<const api::RoadGeometry>> dut =
      CreateRoadGeometryFrom(kFilePath, kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight);
  EXPECT_TRUE(dut.has_value());
  EXPECT_NE(nullptr, dynamic_cast<dragway::RoadGeometry*>(const_cast<maliput::api::RoadGeometry*>(dut.value().get())));
  EXPECT_EQ(nullptr,
            dynamic_cast<multilane::RoadGeometry*>(const_cast<maliput::api::RoadGeometry*>(dut.value().get())));
}

}  // namespace
}  // namespace integration
}  // namespace maliput
