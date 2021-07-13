// Copyright 2020 Toyota Research Institute.
/// @file maliput_derive_lane_s_routes.cc
///
/// Builds an api::RoadGeometry and returns a set of LaneSRoute objects that go from the start waypoint to
/// end one. Possible backends are `dragway`, `multilane` and `malidrive`.
///
/// @note
/// 1. Allows to load a road geometry from different road geometry implementations.
///     The `maliput_backend` flag will determine the backend to be used.
///    - "dragway": The following flags are supported to use in order to create dragway road geometry:
///       -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///    - "multilane": yaml file path must be provided:
///       -yaml_file.
///    - "malidrive": xodr file path must be provided, tolerance and rule file paths are optional:
///         -xodr_file_path -linear_tolerance -road_rule_book_file -traffic_light_book_file -phase_ring_book_file
///         -intersection_book_file
/// 2. Comments about the config_file:
///      i - It should have a valid xodr_file only when malidrive backend is selected.
///     ii - If a xodr_file_path(gflag) is provided then the xodr file path described in the config_file is discarded.
/// 3. The level of the logger could be setted by: -log_level.

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <gflags/gflags.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

#include <maliput/api/lane_data.h>
#include <maliput/api/regions.h>
#include <maliput/api/road_geometry.h>
#include <maliput/api/road_network.h>
#include <maliput/common/logger.h>
#include <maliput/routing/derive_lane_s_routes.h>
#include <maliput/utilities/generate_string.h>
#include <yaml-cpp/yaml.h>

#include <maliput_malidrive/base/inertial_to_lane_mapping_config.h>
#include <maliput_malidrive/constants.h>

using maliput::api::InertialPosition;
using maliput::api::LaneSRoute;
using maliput::api::RoadGeometry;
using maliput::api::RoadGeometryId;
using maliput::api::RoadNetwork;
using maliput::api::RoadPositionResult;
using maliput::routing::DeriveLaneSRoutes;

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "malidrive",
              "Whether to use <dragway>, <multilane> or <malidrive>. Default is malidrive.");
DEFINE_string(config_file, "", "Defines the XODR file, route max length, and waypoints.");
DEFINE_bool(verbose, false,
            "Whether to print the route-derivation input parameters and status "
            "messages. Useful for debugging.");

namespace YAML {

template <>
struct convert<InertialPosition> {
  static Node encode(const InertialPosition& rhs) {
    Node node;
    node.push_back(rhs.x());
    node.push_back(rhs.y());
    node.push_back(rhs.z());
    return node;
  }

  static bool decode(const Node& node, InertialPosition& rhs) {
    if (!node.IsSequence() || node.size() != 3) {
      return false;
    }
    rhs.set_x(node[0].as<double>());
    rhs.set_y(node[1].as<double>());
    rhs.set_z(node[2].as<double>());
    return true;
  }
};

}  // namespace YAML

namespace maliput {
namespace integration {
namespace {

const char* kXodrFileKey = "xodr_file";
const char* kMaxLengthKey = "max_length";
const char* kWaypointKey = "waypoints";
// Distances that differ by less than this (in meters) are considered equal.
constexpr double kDistanceTolerance = 0.01;

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  maliput::common::set_log_level(FLAGS_log_level);

  if (FLAGS_config_file.empty()) {
    maliput::log()->error("No config file specified.");
    return 1;
  }

  const YAML::Node& root_node = YAML::LoadFile(FLAGS_config_file);

  if (!root_node.IsMap()) {
    maliput::log()->error("Invalid YAML file: Root node is not a map.");
    return 1;
  }
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  for (const auto& key : {kXodrFileKey, kMaxLengthKey, kWaypointKey}) {
    if (!root_node[key].IsDefined()) {
      if (key == kXodrFileKey && maliput_implementation != MaliputImplementation::kMalidrive) {
        continue;
      }
      maliput::log()->error("YAML file missing \"{}\".", key);
      return 1;
    }
  }

  log()->info("Loading road network using {} backend implementation...", FLAGS_maliput_backend);
  const std::string xodr_file =
      FLAGS_xodr_file_path.empty() ? root_node[kXodrFileKey].as<std::string>() : FLAGS_xodr_file_path;
  if (maliput_implementation == MaliputImplementation::kMalidrive) {
    maliput::log()->info("xodr file path: {}", xodr_file);
  } else if (maliput_implementation == MaliputImplementation::kMultilane) {
    maliput::log()->info("yaml file path: {}", FLAGS_yaml_file);
  }

  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {xodr_file, FLAGS_linear_tolerance, FLAGS_build_policy, FLAGS_num_threads, FLAGS_simplification_policy,
       FLAGS_tolerance_selection_policy, FLAGS_standard_strictness_policy, FLAGS_omit_nondrivable_lanes,
       FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file});
  log()->info("RoadNetwork loaded successfully.");

  const double max_length = root_node[kMaxLengthKey].as<double>();
  maliput::log()->info("Max length: {}", max_length);

  const YAML::Node& waypoints_node = root_node[kWaypointKey];
  if (!waypoints_node.IsSequence()) {
    maliput::log()->error("Waypoints node is not a sequence.");
    return 1;
  }

  std::vector<InertialPosition> waypoints;
  for (const YAML::Node& waypoint_node : waypoints_node) {
    waypoints.push_back(waypoint_node.as<InertialPosition>());
  }

  maliput::log()->info("Waypoints:");
  for (const auto& waypoint : waypoints) {
    maliput::log()->info("  - {}", waypoint);
  }

  // TODO(liang.fok) Add support for more than two waypoints.
  if (waypoints.size() != 2) {
    maliput::log()->error("Currently, only two waypoints are supported.");
    return 1;
  }

  const InertialPosition& start_inertial = waypoints.front();
  const InertialPosition& end_inertial = waypoints.back();
  const double start_end_dist = (start_inertial - end_inertial).length();
  if (start_end_dist > max_length) {
    maliput::log()->error("Distance between first and last waypoint ({})  exceeds max length ({}).", start_end_dist,
                          max_length);
    return 1;
  }

  const RoadGeometry* road_geometry = rn->road_geometry();
  const RoadPositionResult start = road_geometry->ToRoadPosition(start_inertial);
  const RoadPositionResult end = road_geometry->ToRoadPosition(end_inertial);

  maliput::log()->info("Start RoadPosition:");
  maliput::log()->info("  - Lane: {}", start.road_position.lane->id().string());
  maliput::log()->info("  - s,r,h: ({}, {}, {})", start.road_position.pos.s(), start.road_position.pos.r(),
                       start.road_position.pos.h());
  maliput::log()->info("End RoadPosition:");
  maliput::log()->info("  - Lane: {}", end.road_position.lane->id().string());
  maliput::log()->info("  - s,r,h: ({}, {}, {})", end.road_position.pos.s(), end.road_position.pos.r(),
                       end.road_position.pos.h());

  const std::vector<LaneSRoute> routes = DeriveLaneSRoutes(start.road_position, end.road_position, max_length);

  maliput::log()->info("Number of routes: {}", routes.size());

  if (routes.empty()) {
    maliput::log()->error("No routes found.");
    return 1;
  }

  std::stringstream buffer;
  for (size_t i = 0; i < routes.size(); ++i) {
    const auto& route = routes.at(i);
    YAML::Node route_node;
    for (const auto& range : route.ranges()) {
      YAML::Node range_node;
      range_node["Lane"] = range.lane_id().string();
      const double s0 = range.s_range().s0();
      const double s1 = range.s_range().s1();
      const double lane_length = road_geometry->ById().GetLane(range.lane_id())->length();
      const double lane_length_delta = std::abs(std::abs(s1 - s0) - lane_length);
      if (FLAGS_verbose) {
        std::cout << "Lane " << range.lane_id().string() << ", |s1 - s0| = " << std::abs(s1 - s0)
                  << ", lane length = " << lane_length << ", delta = " << lane_length_delta << std::endl;
      }
      if (lane_length_delta > kDistanceTolerance) {
        YAML::Node s_range_node;
        s_range_node.SetStyle(YAML::EmitterStyle::Flow);
        s_range_node.push_back(s0);
        s_range_node.push_back(s1);
        range_node["SRange"] = s_range_node;
      }
      route_node.push_back(range_node);
    }
    YAML::Emitter emitter;
    emitter << route_node;
    buffer << "Route " << (i + 1) << " of " << routes.size() << ":\n";
    buffer << emitter.c_str();
    if (i < routes.size() - 1) {
      buffer << "\n";
    }
  }

  maliput::log()->info(buffer.str());
  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::main(argc, argv); }
