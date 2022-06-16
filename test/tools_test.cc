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

constexpr char kYamlFileName[] = "2x2_intersection.yaml";
constexpr char kYamlFileRoadGeometryId[] = "basic_two_lane_x_intersection";
constexpr char kXodrFileName[] = "ArcLane.xodr";
constexpr int kNumLanes{2};
constexpr double kLength{10.};
constexpr double kLaneWidth{3.7};
constexpr double kShoulderWidth{3.};
constexpr double kMaximumHeight{5.2};

constexpr char MALIPUT_MALIDRIVE_RESOURCE_ROOT[] = "MALIPUT_MALIDRIVE_RESOURCE_ROOT";
constexpr char MULTILANE_RESOURCE_ROOT[] = "MULTILANE_RESOURCE_ROOT";

GTEST_TEST(CreateRoadNetwork, MalidriveRoadNetwork) {
  setenv(MALIPUT_MALIDRIVE_RESOURCE_ROOT, DEF_MALIDRIVE_RESOURCES, 1);
  std::unique_ptr<const api::RoadNetwork> dut = CreateMalidriveRoadNetwork({kXodrFileName, 5e-2});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  // To evaluate the following condition, malidrive::RoadGeometry type is required. It is defined
  // in maliput_malidrive/base/road_geometry.h but it brings with it a lot of header files that
  // should not be exported.
  // EXPECT_NE(nullptr, dynamic_cast<const malidrive::RoadGeometry*>(dut->road_geometry()));
}

GTEST_TEST(CreateRoadNetwork, MultilaneRoadNetwork) {
  setenv(MULTILANE_RESOURCE_ROOT, DEF_MULTILANE_RESOURCES, 1);
  std::unique_ptr<const api::RoadNetwork> dut = CreateMultilaneRoadNetwork({kYamlFileName});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  EXPECT_EQ(dut->road_geometry()->id(), maliput::api::RoadGeometryId{kYamlFileRoadGeometryId});
}

GTEST_TEST(CreateRoadNetwork, DragwayRoadNetwork) {
  std::unique_ptr<const api::RoadNetwork> dut =
      CreateDragwayRoadNetwork(DragwayBuildProperties{kNumLanes, kLength, kLaneWidth, kShoulderWidth, kMaximumHeight});
  EXPECT_NE(nullptr, dut);
  EXPECT_NE(nullptr, dut->road_geometry());
  EXPECT_NE(nullptr, dynamic_cast<const dragway::RoadGeometry*>(dut->road_geometry()));
}

}  // namespace
}  // namespace integration
}  // namespace maliput
