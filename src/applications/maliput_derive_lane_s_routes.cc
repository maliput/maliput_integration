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
/// 3. The log level could be setted by: -log_level.

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <maliput/api/lane_data.h>
#include <maliput/api/regions.h>
#include <maliput/api/road_geometry.h>
#include <maliput/api/road_network.h>
#include <maliput/common/logger.h>
#include <maliput/routing/derive_lane_s_routes.h>
#include <maliput/utility/generate_string.h>
#include <yaml-cpp/yaml.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

using maliput::api::InertialPosition;
using maliput::api::LaneSRoute;
using maliput::api::RoadGeometry;
using maliput::api::RoadGeometryId;
using maliput::api::RoadNetwork;
using maliput::api::RoadPositionResult;
using maliput::routing::DeriveLaneSRoutes;

COMMON_PROPERTIES_FLAGS();
MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_OSM_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "malidrive",
              "Whether to use <dragway>, <multilane> or <malidrive>. Default is malidrive.");
DEFINE_string(config_file, "", "YAML file that defines XODR file path, route max length, and start/end waypoints.");
DEFINE_double(max_length, 1000, "Maximum length of the intermediate lanes between start and end waypoints.[m]");
DEFINE_string(start_waypoint, "", "Start waypoint to calculate the routing from. Expected format: '{x0, y0, z0}' ");
DEFINE_string(end_waypoint, "", "End waypoint to calculate the routing to. Expected format: '{x1, y1, z1}' ");

namespace YAML {

template <>
struct convert<maliput::math::Vector3> {
  static Node encode(const maliput::math::Vector3& rhs) {
    Node node;
    node.push_back(rhs.x());
    node.push_back(rhs.y());
    node.push_back(rhs.z());
    return node;
  }

  static bool decode(const Node& node, maliput::math::Vector3& rhs) {
    if (!node.IsSequence() || node.size() != 3) {
      return false;
    }
    rhs.x() = node[0].as<double>();
    rhs.y() = node[1].as<double>();
    rhs.z() = node[2].as<double>();
    return true;
  }
};

}  // namespace YAML

namespace maliput {
namespace integration {
namespace {

constexpr const char* kXodrFileKey = "xodr_file";
constexpr const char* kYamlFileKey = "yaml_file";
constexpr const char* kMaxLengthKey = "max_length";
constexpr const char* kWaypointKey = "waypoints";
// Distances that differ by less than this (in meters) are considered equal.
constexpr double kDistanceTolerance = 0.01;

// Derives and returns a set of LaneSRoute objects that go from @p start to
// @p end . If no routes are found, a vector of length zero is returned.
// Parameter @p max_length is the maximum length of the intermediate lanes
// between @p start and @p end. See the description of maliput::routing::DeriveLaneSRoutes() for
// more details. If @p start and @p end are the same lane, a route consisting
// of one lane is returned regardless of @p max_length.
std::vector<LaneSRoute> GetRoutes(const InertialPosition& start, const InertialPosition& end, const double max_length,
                                  const RoadGeometry* road_geometry) {
  const RoadPositionResult start_rp = road_geometry->ToRoadPosition(start);
  const RoadPositionResult end_rp = road_geometry->ToRoadPosition(end);

  maliput::log()->info("Start RoadPosition:");
  maliput::log()->info("  - Lane: ", start_rp.road_position.lane->id().string());
  maliput::log()->info("  - s,r,h: (", start_rp.road_position.pos.s(), ", ", start_rp.road_position.pos.r(), ", ",
                       start_rp.road_position.pos.h(), ")");
  maliput::log()->info("End RoadPosition:");
  maliput::log()->info("  - Lane: ", end_rp.road_position.lane->id().string());
  maliput::log()->info("  - s,r,h: (", end_rp.road_position.pos.s(), ", ", end_rp.road_position.pos.r(), ", ",
                       end_rp.road_position.pos.h(), ")");

  return DeriveLaneSRoutes(start_rp.road_position, end_rp.road_position, max_length);
}

// Serializes the @p routes computed by using the GetRoutes() method into a std::string.
std::string SerializeLaneSRoutes(const std::vector<LaneSRoute>& routes, const RoadGeometry* road_geometry) {
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
      maliput::log()->trace("Lane ", range.lane_id().string(), ", |s1 - s0| = ", std::abs(s1 - s0),
                            ", lane length = ", lane_length, ", delta = ", lane_length_delta);
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
  return buffer.str();
}

// Resolves the configuration parameters. Routing configuration can be loaded by using a configuration file or gflags.
// @param[in] maliput_implementation Selected maliput backend.
// @param[in] flag_config_file Configuration file path passed as gflags to the app.
// @param[in] flag_xodr_file_path If backend is malidrive, the XODR file passed as gflags to the app.
// @param[in] flag_yaml_file YAML If backend is multilane, the YAML file passed as gflags to the app.
// @param[in] flag_start_waypoint Start waypoint passed as gflags to the app.
// @param[in] flag_end_waypoint End waypoint passed as gflags to the app.
// @param[in] flag_max_length Max_length passed as gflags to the app.
// @param[out] waypoints Start and end waypoints to be used.
// @param[out] max_length Max length to be used.
// @param[out] xodr_file XODR file path to be used when using malidrive backend.
// @param[out] yaml_file YAML file path to be used when using multilane backend.
// @returns False when parameters can't be resolved, True otherwise.
bool ResolveConfigFields(const MaliputImplementation& maliput_implementation, const std::string& flag_config_file,
                         const std::string& flag_xodr_file_path, const std::string& flag_yaml_file,
                         const std::string& flag_start_waypoint, const std::string& flag_end_waypoint,
                         const double flag_max_length, std::vector<maliput::math::Vector3>& waypoints,
                         double& max_length, std::string& xodr_file, std::string& yaml_file) {
  // If configuration file is passed, check the YAML fields.
  if (!FLAGS_config_file.empty()) {
    maliput::log()->info("Configuration file is passed: ", FLAGS_config_file);
    const YAML::Node& root_node = YAML::LoadFile(FLAGS_config_file);
    if (!root_node.IsMap()) {
      maliput::log()->error("Invalid YAML file: Root node is not a map.");
      return false;
    }
    for (const auto& key : {kXodrFileKey, kMaxLengthKey, kWaypointKey, kYamlFileKey}) {
      if (!root_node[key].IsDefined()) {
        if (key == kXodrFileKey && maliput_implementation != MaliputImplementation::kMalidrive) {
          continue;
        }
        if (key == kYamlFileKey && maliput_implementation != MaliputImplementation::kMultilane) {
          continue;
        }
        maliput::log()->error("YAML file missing \"", key, "\".");
        return false;
      }
    }
    // Get map file path if necessary.
    if (maliput_implementation == MaliputImplementation::kMalidrive) {
      xodr_file = root_node[kXodrFileKey].as<std::string>();
    } else if (maliput_implementation == MaliputImplementation::kMultilane) {
      yaml_file = root_node[kYamlFileKey].as<std::string>();
    }
    // Get max_length from config file.
    max_length = root_node[kMaxLengthKey].as<double>();

    // Get waypoints from config file.
    const YAML::Node& waypoints_node = root_node[kWaypointKey];
    if (!waypoints_node.IsSequence()) {
      maliput::log()->error("Waypoints node is not a sequence.");
      return false;
    }
    for (const YAML::Node& waypoint_node : waypoints_node) {
      waypoints.push_back(waypoint_node.as<maliput::math::Vector3>());
    }
    if (waypoints.size() != 2) {
      maliput::log()->error("Currently, only two waypoints are supported.");
      return false;
    }
  } else {
    maliput::log()->info("Configuration loaded from flags as configuration file flag isn't used.");
    // Get XODR/YAML files from flags if correspond.
    if (maliput_implementation == MaliputImplementation::kMalidrive) {
      if (FLAGS_xodr_file_path.empty()) {
        maliput::log()->error(
            "For malidrive backend, '--xodr_file_path' flag must be used when configuration file is missing.");
        return false;
      }
      xodr_file = FLAGS_xodr_file_path;
    } else if (maliput_implementation == MaliputImplementation::kMultilane) {
      if (FLAGS_yaml_file.empty()) {
        maliput::log()->error(
            "For multilane backend, '--yaml_file' flag must be used when configuration file is missing.");
        return false;
      }
      yaml_file = FLAGS_yaml_file;
    }

    // Get max_length from flags.
    max_length = flag_max_length;

    // Get waypoints from flags.
    if (FLAGS_start_waypoint.empty()) {
      maliput::log()->error("'--start_waypoint; flag must be used when configuration file is missing.");
      return false;
    }
    waypoints.push_back(maliput::math::Vector3::FromStr(FLAGS_start_waypoint));
    if (FLAGS_end_waypoint.empty()) {
      maliput::log()->error("'--end_waypoint; flag must be used when configuration file is missing.");
      return false;
    }
    waypoints.push_back(maliput::math::Vector3::FromStr(FLAGS_end_waypoint));
  }
  return true;
}

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  maliput::common::set_log_level(FLAGS_log_level);

  // Get maliput implementation: Dragway, Malidrive or Multilane.
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};

  // Initialize configuration fields.
  std::vector<maliput::math::Vector3> waypoints;
  double max_length;
  std::string xodr_file{""};
  std::string yaml_file{""};

  if (!ResolveConfigFields(maliput_implementation, FLAGS_config_file, FLAGS_xodr_file_path, FLAGS_yaml_file,
                           FLAGS_start_waypoint, FLAGS_end_waypoint, FLAGS_max_length, waypoints, max_length, xodr_file,
                           yaml_file)) {
    return 1;
  }

  maliput::log()->info("Max length: ", max_length);
  maliput::log()->info("Waypoints:");
  for (const auto& waypoint : waypoints) {
    maliput::log()->info("  - ", waypoint);
  }

  maliput::log()->info("Loading road network using ", FLAGS_maliput_backend, " backend implementation...");

  if (maliput_implementation == MaliputImplementation::kMalidrive) {
    maliput::log()->info("xodr file path: ", xodr_file);
  } else if (maliput_implementation == MaliputImplementation::kMultilane) {
    maliput::log()->info("yaml file path: ", yaml_file);
  }

  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {yaml_file},
      {xodr_file, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), GetAngularToleranceFlag(), FLAGS_build_policy,
       FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy, FLAGS_omit_nondrivable_lanes,
       FLAGS_integrator_accuracy_multiplier, FLAGS_rule_registry_file, FLAGS_road_rule_book_file,
       FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file, FLAGS_intersection_book_file},
      {FLAGS_osm_file, FLAGS_linear_tolerance, FLAGS_angular_tolerance, maliput::math::Vector2::FromStr(FLAGS_origin),
       FLAGS_rule_registry_file, FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file});
  log()->info("RoadNetwork loaded successfully.");

  const RoadGeometry* road_geometry = rn->road_geometry();
  const std::vector<LaneSRoute> routes =
      GetRoutes(InertialPosition::FromXyz(waypoints.front()), InertialPosition::FromXyz(waypoints.back()), max_length,
                road_geometry);

  maliput::log()->info("Number of routes: ", routes.size());

  if (routes.empty()) {
    maliput::log()->error("No routes found.");
    return 1;
  }

  std::cout << SerializeLaneSRoutes(routes, road_geometry) << std::endl;
  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::main(argc, argv); }
