// BSD 3-Clause License
//
// Copyright (c) 2022, Woven Planet. All rights reserved.
// Copyright (c) 2022, Toyota Research Institute. All rights reserved.
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

#include <map>

#include <maliput/base/intersection_book.h>
#include <maliput/base/intersection_book_loader.h>
#include <maliput/base/manual_discrete_value_rule_state_provider.h>
#include <maliput/base/manual_phase_provider.h>
#include <maliput/base/manual_phase_ring_book.h>
#include <maliput/base/manual_range_value_rule_state_provider.h>
#include <maliput/base/manual_right_of_way_rule_state_provider.h>
#include <maliput/base/manual_rulebook.h>
#include <maliput/base/phase_ring_book_loader.h>
#include <maliput/base/road_rulebook_loader.h>
#include <maliput/base/traffic_light_book.h>
#include <maliput/base/traffic_light_book_loader.h>
#include <maliput/common/filesystem.h>
#include <maliput/common/logger.h>
#include <maliput/common/maliput_abort.h>
#include <maliput_dragway/road_geometry.h>
#include <maliput_malidrive/builder/road_network_builder.h>
#include <maliput_malidrive/constants.h>
#include <maliput_malidrive/loader/loader.h>
#include <maliput_multilane/builder.h>
#include <maliput_multilane/loader.h>
#include <maliput_multilane/road_network_builder.h>
#include <maliput_osm/builder/road_network_builder.h>
#include <yaml-cpp/yaml.h>

namespace maliput {
namespace integration {
namespace {

constexpr char MALIPUT_MALIDRIVE_RESOURCE_ROOT[] = "MALIPUT_MALIDRIVE_RESOURCE_ROOT";
constexpr char MULTILANE_RESOURCE_ROOT[] = "MULTILANE_RESOURCE_ROOT";
constexpr char MALIPUT_OSM_RESOURCE_ROOT[] = "MALIPUT_OSM_RESOURCE_ROOT";

// Holds the conversions from MaliputImplementation to std::string.
const std::map<MaliputImplementation, std::string> maliput_impl_to_string{
    {MaliputImplementation::kDragway, "dragway"},
    {MaliputImplementation::kMalidrive, "malidrive"},
    {MaliputImplementation::kMultilane, "multilane"},
    {MaliputImplementation::kOsm, "osm"},
};

// Holds the conversions from std::string to MaliputImplementation.
const std::map<std::string, MaliputImplementation> string_to_maliput_impl{
    {"dragway", MaliputImplementation::kDragway},
    {"malidrive", MaliputImplementation::kMalidrive},
    {"multilane", MaliputImplementation::kMultilane},
    {"osm", MaliputImplementation::kOsm},
};

// @returns @p file_name 's path located at @p env path. If not located, an empty string is returned.
// @throws maliput::common::assertion_error When @p file_name is an absolute path.
std::string GetFilePathFromEnv(const std::string& file_name, const std::string& env) {
  MALIPUT_THROW_UNLESS(!maliput::common::Path{file_name}.is_absolute());
  maliput::common::Path file_path = maliput::common::Filesystem::get_env_path(env);
  file_path.append(file_name);
  // Returns empty string if file_path doesn't match any existing file.
  return file_path.exists() ? file_path.get_path() : "";
}

}  // namespace

std::string MaliputImplementationToString(MaliputImplementation maliput_impl) {
  return maliput_impl_to_string.at(maliput_impl);
}

MaliputImplementation StringToMaliputImplementation(const std::string& maliput_impl) {
  MALIPUT_DEMAND(string_to_maliput_impl.find(maliput_impl) != string_to_maliput_impl.end());
  return string_to_maliput_impl.at(maliput_impl);
}

std::unique_ptr<api::RoadNetwork> CreateDragwayRoadNetwork(const DragwayBuildProperties& build_properties) {
  maliput::log()->debug("Building dragway RoadNetwork.");
  auto rg = std::make_unique<dragway::RoadGeometry>(
      api::RoadGeometryId{"Dragway with " + std::to_string(build_properties.num_lanes) + " lanes."},
      build_properties.num_lanes, build_properties.length, build_properties.lane_width, build_properties.shoulder_width,
      build_properties.maximum_height, std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon(),
      maliput::math::Vector3(0, 0, 0));

  std::unique_ptr<ManualRulebook> rulebook = std::make_unique<ManualRulebook>();
  std::unique_ptr<TrafficLightBook> traffic_light_book = std::make_unique<TrafficLightBook>();
  std::unique_ptr<api::rules::RuleRegistry> rule_registry = std::make_unique<api::rules::RuleRegistry>();
  std::unique_ptr<ManualPhaseRingBook> phase_ring_book = std::make_unique<ManualPhaseRingBook>();
  std::unique_ptr<ManualPhaseProvider> phase_provider = std::make_unique<ManualPhaseProvider>();
  std::unique_ptr<IntersectionBook> intersection_book = std::make_unique<IntersectionBook>(rg.get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  std::unique_ptr<ManualRightOfWayRuleStateProvider> right_of_way_rule_state_provider =
      std::make_unique<ManualRightOfWayRuleStateProvider>();
#pragma GCC diagnostic pop
  std::unique_ptr<ManualDiscreteValueRuleStateProvider> discrete_value_rule_state_provider =
      std::make_unique<ManualDiscreteValueRuleStateProvider>(rulebook.get());
  std::unique_ptr<ManualRangeValueRuleStateProvider> range_value_rule_state_provider =
      std::make_unique<ManualRangeValueRuleStateProvider>(rulebook.get());
  return std::make_unique<api::RoadNetwork>(std::move(rg), std::move(rulebook), std::move(traffic_light_book),
                                            std::move(intersection_book), std::move(phase_ring_book),
                                            std::move(right_of_way_rule_state_provider), std::move(phase_provider),
                                            std::move(rule_registry), std::move(discrete_value_rule_state_provider),
                                            std::move(range_value_rule_state_provider));
}

std::unique_ptr<api::RoadNetwork> CreateMultilaneRoadNetwork(const MultilaneBuildProperties& build_properties) {
  maliput::log()->debug("Building multilane RoadNetwork.");
  if (build_properties.yaml_file.empty()) {
    MALIPUT_ABORT_MESSAGE("yaml_file cannot be empty.");
  }
  const std::string yaml_file_path = GetResource(MaliputImplementation::kMultilane, build_properties.yaml_file);
  maliput::multilane::RoadNetworkConfiguration config;
  config.yaml_file = yaml_file_path;
  return maliput::multilane::BuildRoadNetwork(config);
}

std::unique_ptr<api::RoadNetwork> CreateMalidriveRoadNetwork(const MalidriveBuildProperties& build_properties) {
  maliput::log()->debug("Building malidrive RoadNetwork.");
  MALIPUT_VALIDATE(!build_properties.xodr_file_path.empty(), "opendrive_file cannot be empty.");

  std::map<std::string, std::string> road_network_configuration;
  road_network_configuration.emplace("road_geometry_id", "malidrive_rg");
  road_network_configuration.emplace("opendrive_file",
                                     GetResource(MaliputImplementation::kMalidrive, build_properties.xodr_file_path));
  if (build_properties.linear_tolerance.has_value()) {
    road_network_configuration.emplace("linear_tolerance", std::to_string(build_properties.linear_tolerance.value()));
  }
  if (build_properties.max_linear_tolerance.has_value()) {
    road_network_configuration.emplace("max_linear_tolerance",
                                       std::to_string(build_properties.max_linear_tolerance.value()));
  }
  if (build_properties.angular_tolerance.has_value()) {
    road_network_configuration.emplace("angular_tolerance", std::to_string(build_properties.angular_tolerance.value()));
  }
  road_network_configuration.emplace("scale_length", std::to_string(malidrive::constants::kScaleLength));
  road_network_configuration.emplace("inertial_to_backend_frame_translation", "{0., 0., 0.}");
  road_network_configuration.emplace("build_policy", build_properties.build_policy);
  if (build_properties.number_of_threads != 0) {
    road_network_configuration.emplace("num_threads", std::to_string(build_properties.number_of_threads));
  }
  road_network_configuration.emplace("simplification_policy", build_properties.simplification_policy);
  road_network_configuration.emplace("standard_strictness_policy", build_properties.standard_strictness_policy);
  road_network_configuration.emplace("omit_nondrivable_lanes",
                                     build_properties.omit_nondrivable_lanes ? "true" : "false");
  road_network_configuration.emplace("integrator_accuracy_multiplier",
                                     std::to_string(build_properties.integrator_accuracy_multiplier));
  road_network_configuration.emplace("use_userdata_traffic_direction",
                                     build_properties.use_userdata_traffic_direction ? "true" : "false");
  if (!build_properties.rule_registry_file.empty()) {
    road_network_configuration.emplace(
        "rule_registry", GetResource(MaliputImplementation::kMalidrive, build_properties.rule_registry_file));
  }
  if (!build_properties.road_rule_book_file.empty()) {
    road_network_configuration.emplace(
        "road_rule_book", GetResource(MaliputImplementation::kMalidrive, build_properties.road_rule_book_file));
  }
  if (!build_properties.traffic_light_book_file.empty()) {
    road_network_configuration.emplace(
        "traffic_light_book", GetResource(MaliputImplementation::kMalidrive, build_properties.traffic_light_book_file));
  }
  if (!build_properties.phase_ring_book_file.empty()) {
    road_network_configuration.emplace(
        "phase_ring_book", GetResource(MaliputImplementation::kMalidrive, build_properties.phase_ring_book_file));
  }
  if (!build_properties.intersection_book_file.empty()) {
    road_network_configuration.emplace(
        "intersection_book", GetResource(MaliputImplementation::kMalidrive, build_properties.intersection_book_file));
  }

  return malidrive::loader::Load<malidrive::builder::RoadNetworkBuilder>(road_network_configuration);
}

std::unique_ptr<api::RoadNetwork> CreateMaliputOsmRoadNetwork(const MaliputOsmBuildProperties& build_properties) {
  maliput::log()->debug("Building maliput_osm RoadNetwork.");
  MALIPUT_VALIDATE(!build_properties.osm_file.empty(), "osm_file cannot be empty.");

  std::map<std::string, std::string> build_configuration;
  build_configuration.emplace("road_geometry_id", "maliput_osm_rg");
  build_configuration.emplace("osm_file", GetResource(MaliputImplementation::kOsm, build_properties.osm_file));
  build_configuration.emplace("linear_tolerance", std::to_string(build_properties.linear_tolerance));
  build_configuration.emplace("angular_tolerance", std::to_string(build_properties.angular_tolerance));
  build_configuration.emplace("inertial_to_backend_frame_translation", "{0., 0., 0.}");
  build_configuration.emplace("origin", build_properties.origin.to_str());
  if (!build_properties.rule_registry_file.empty()) {
    build_configuration.emplace("rule_registry",
                                GetResource(MaliputImplementation::kOsm, build_properties.rule_registry_file));
  }
  if (!build_properties.road_rule_book_file.empty()) {
    build_configuration.emplace("road_rule_book",
                                GetResource(MaliputImplementation::kOsm, build_properties.road_rule_book_file));
  }
  if (!build_properties.traffic_light_book_file.empty()) {
    build_configuration.emplace("traffic_light_book",
                                GetResource(MaliputImplementation::kOsm, build_properties.traffic_light_book_file));
  }
  if (!build_properties.phase_ring_book_file.empty()) {
    build_configuration.emplace("phase_ring_book",
                                GetResource(MaliputImplementation::kOsm, build_properties.phase_ring_book_file));
  }
  if (!build_properties.intersection_book_file.empty()) {
    build_configuration.emplace("intersection_book",
                                GetResource(MaliputImplementation::kOsm, build_properties.intersection_book_file));
  }

  return maliput_osm::builder::RoadNetworkBuilder(build_configuration)();
}

std::unique_ptr<api::RoadNetwork> LoadRoadNetwork(MaliputImplementation maliput_implementation,
                                                  const DragwayBuildProperties& dragway_build_properties,
                                                  const MultilaneBuildProperties& multilane_build_properties,
                                                  const MalidriveBuildProperties& malidrive_build_properties,
                                                  const MaliputOsmBuildProperties& maliput_osm_build_properties) {
  switch (maliput_implementation) {
    case MaliputImplementation::kDragway:
      return CreateDragwayRoadNetwork(dragway_build_properties);
    case MaliputImplementation::kMultilane:
      return CreateMultilaneRoadNetwork(multilane_build_properties);
    case MaliputImplementation::kMalidrive:
      return CreateMalidriveRoadNetwork(malidrive_build_properties);
    case MaliputImplementation::kOsm:
      return CreateMaliputOsmRoadNetwork(maliput_osm_build_properties);
    default:
      MALIPUT_ABORT_MESSAGE("Error loading RoadNetwork. Unknown implementation.");
  }
}

std::string GetResource(const MaliputImplementation& maliput_implementation, const std::string& resource_name) {
  std::string file_path{""};
  switch (maliput_implementation) {
    case MaliputImplementation::kMalidrive:
      file_path = GetFilePathFromEnv("resources/odr/" + resource_name, MALIPUT_MALIDRIVE_RESOURCE_ROOT);
      break;
    case MaliputImplementation::kMultilane:
      file_path = GetFilePathFromEnv(resource_name, MULTILANE_RESOURCE_ROOT);
      break;
    case MaliputImplementation::kOsm:
      file_path = GetFilePathFromEnv("resources/osm/" + resource_name, MALIPUT_OSM_RESOURCE_ROOT);
      break;
    default:
      break;
  }
  return file_path.empty() ? resource_name : file_path;
}

}  // namespace integration
}  // namespace maliput
