#include <gtest/gtest.h>

#include "integration/tools.h"

#include "maliput/common/filesystem.h"

#include "maliput_dragway/road_geometry.h"

#include "maliput_malidrive/base/road_geometry.h"

#include "maliput_multilane/builder.h"
#include "maliput_multilane/loader.h"
#include "maliput_multilane/road_geometry.h"

namespace maliput {
namespace integration {
namespace {

constexpr char MULTILANE_RESOURCE_VAR[] = "MULTILANE_RESOURCE_ROOT";
constexpr char MALIPUT_MALIDRIVE_RESOURCE_VAR[] = "MALIPUT_MALIDRIVE_RESOURCE_ROOT";
constexpr char kYamlFileName[] = "/2x2_intersection.yaml";
constexpr char kXodrFileName[] = "/resources/odr/ArcLane.xodr";
constexpr int kNumLanes{2};
constexpr double kLength{10.};
constexpr double kLaneWidth{3.7};
constexpr double kShoulderWidth{3.};
constexpr double kMaximumHeight{5.2};

GTEST_TEST(CreateRoadGeometry, MalidriveRoadGeometry) {
  const std::string kFilePath{maliput::common::Filesystem::get_env_path(MALIPUT_MALIDRIVE_RESOURCE_VAR) +
                              kXodrFileName};
  std::unique_ptr<const api::RoadGeometry> dut = CreateMalidriveRoadGeometry({kFilePath, 5e-2});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dynamic_cast<const malidrive::RoadGeometry*>(dut.get()));
}

GTEST_TEST(CreateRoadGeometry, MultilaneRoadGeometry) {
  const std::string kFilePath{maliput::common::Filesystem::get_env_path(MULTILANE_RESOURCE_VAR) + kYamlFileName};
  std::unique_ptr<const api::RoadGeometry> dut = CreateMultilaneRoadGeometry({kFilePath});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dynamic_cast<const multilane::RoadGeometry*>(dut.get()));
}

GTEST_TEST(CreateRoadGeometry, DragwayRoadGeometry) {
  std::unique_ptr<const api::RoadGeometry> dut =
      CreateDragwayRoadGeometry(DragwayBuildProperties{kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dynamic_cast<const dragway::RoadGeometry*>(dut.get()));
}

}  // namespace
}  // namespace integration
}  // namespace maliput
