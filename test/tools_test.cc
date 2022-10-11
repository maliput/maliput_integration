// BSD 3-Clause License
//
// Copyright (c) 2022, Woven Planet. All rights reserved.
// Copyright (c) 2020-2022, Toyota Research Institute. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "integration/tools.h"

#include <stdlib.h>

#include <gtest/gtest.h>
#include <maliput_dragway/road_geometry.h>
#include <maliput_multilane/builder.h>
#include <maliput_multilane/loader.h>

namespace maliput {
namespace integration {
namespace {

class CreateMalidriveRoadNetworkTest : public ::testing::Test {
 public:
  static constexpr int kOverwrite{1};
  static constexpr char MALIPUT_MALIDRIVE_RESOURCE_ROOT[] = "MALIPUT_MALIDRIVE_RESOURCE_ROOT";

  void SetUp() override {
    const auto env = getenv(MALIPUT_MALIDRIVE_RESOURCE_ROOT);
    if (env != NULL) env_back_up_ = env;
    setenv(MALIPUT_MALIDRIVE_RESOURCE_ROOT, DEF_MALIDRIVE_RESOURCES, kOverwrite);
  }
  void TearDown() override {
    unsetenv(MALIPUT_MALIDRIVE_RESOURCE_ROOT);
    if (!env_back_up_.empty()) setenv(MALIPUT_MALIDRIVE_RESOURCE_ROOT, env_back_up_.c_str(), kOverwrite);
  }

 private:
  std::string env_back_up_{};
};

TEST_F(CreateMalidriveRoadNetworkTest, MalidriveRoadNetwork) {
  static constexpr char kXodrFileName[] = "ArcLane.xodr";
  std::unique_ptr<const api::RoadNetwork> dut = CreateMalidriveRoadNetwork({kXodrFileName, 5e-2});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  // To evaluate the following condition, malidrive::RoadGeometry type is required. It is defined
  // in maliput_malidrive/base/road_geometry.h but it brings with it a lot of header files that
  // should not be exported.
  // EXPECT_NE(nullptr, dynamic_cast<const malidrive::RoadGeometry*>(dut->road_geometry()));
}

class CreateMultilaneRoadNetworkTest : public ::testing::Test {
 public:
  static constexpr int kOverwrite{1};
  static constexpr char MULTILANE_RESOURCE_ROOT[] = "MULTILANE_RESOURCE_ROOT";

  void SetUp() override {
    const auto env = getenv(MULTILANE_RESOURCE_ROOT);
    if (env != NULL) env_back_up_ = env;
    setenv(MULTILANE_RESOURCE_ROOT, DEF_MULTILANE_RESOURCES, kOverwrite);
  }
  void TearDown() override {
    unsetenv(MULTILANE_RESOURCE_ROOT);
    if (!env_back_up_.empty()) setenv(MULTILANE_RESOURCE_ROOT, env_back_up_.c_str(), kOverwrite);
  }

 private:
  std::string env_back_up_{};
};

TEST_F(CreateMultilaneRoadNetworkTest, MultilaneRoadNetwork) {
  static constexpr char kYamlFileName[] = "2x2_intersection.yaml";
  static constexpr char kYamlFileRoadGeometryId[] = "basic_two_lane_x_intersection";
  std::unique_ptr<const api::RoadNetwork> dut = CreateMultilaneRoadNetwork({kYamlFileName});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  EXPECT_EQ(dut->road_geometry()->id(), maliput::api::RoadGeometryId{kYamlFileRoadGeometryId});
}

class CreateDragwayRoadNetworkTest : public ::testing::Test {};

GTEST_TEST(CreateRoadNetwork, DragwayRoadNetwork) {
  static constexpr int kNumLanes{2};
  static constexpr double kLength{10.};
  static constexpr double kLaneWidth{3.7};
  static constexpr double kShoulderWidth{3.};
  static constexpr double kMaximumHeight{5.2};
  std::unique_ptr<const api::RoadNetwork> dut =
      CreateDragwayRoadNetwork(DragwayBuildProperties{kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  EXPECT_NE(nullptr, dynamic_cast<const dragway::RoadGeometry*>(dut->road_geometry()));
}

class CreateMaliputOsmRoadNetworkTest : public ::testing::Test {
 public:
  static constexpr int kOverwrite{1};
  static constexpr char MULTILANE_RESOURCE_ROOT[] = "MULTILANE_RESOURCE_ROOT";

  void SetUp() override {
    const auto env = getenv(MULTILANE_RESOURCE_ROOT);
    if (env != NULL) env_back_up_ = env;
    setenv(MULTILANE_RESOURCE_ROOT, DEF_MULTILANE_RESOURCES, kOverwrite);
  }
  void TearDown() override {
    unsetenv(MULTILANE_RESOURCE_ROOT);
    if (!env_back_up_.empty()) setenv(MULTILANE_RESOURCE_ROOT, env_back_up_.c_str(), kOverwrite);
  }

 private:
  std::string env_back_up_{};
};

TEST_F(CreateMaliputOsmRoadNetworkTest, MaliputOsmRoadNetwork) {
  static constexpr char kFileName[] = "straight_forward.osm";
  static constexpr char kRoadGeometryId[] = "maliput_osm_rg";
  std::unique_ptr<const api::RoadNetwork> dut = CreateMaliputOsmRoadNetwork({kFileName, 1e-3, 1e-3, {0., 0.}});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  EXPECT_EQ(dut->road_geometry()->id(), maliput::api::RoadGeometryId{kRoadGeometryId});
}

}  // namespace
}  // namespace integration
}  // namespace maliput
