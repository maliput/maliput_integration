// Copyright 2021 Toyota Research Institute.

/// @file maliput_to_string_with_plugin.cc
///
/// Builds an api::RoadNetwork and lists its entities.
/// The road network is created using the maliput plugin architecture.
///
/// Available backends are `dragway`, `multilane` and `malidrive`, flags are provided
/// to correctly configure the requested paramteres for building the road network.
/// @see maliput::plugin::MaliputPluginManager
///
/// @note
///   1. The `plugin_name` flag will determine the maliput::plugin::RoadNetworkLoader plugin to be selected.
///       - "maliput_dragway": The following flags are supported to use in order to create dragway road geometry:
///           -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///       - "maliput_multilane": yaml file path must be provided:
///           -yaml_file.
///       - "maliput_malidrive": xodr file path must be provided, the tolerances and scale length are optional:
///           -xodr_file_path, -linear_tolerance, -angular_tolerance, -scale_length.
///   2. The applications possesses flags to modify the output serialization:
///      -include_type_labels, -include_road_geometry_id, -include_junction_ids,
///      -include_segment_ids, -include_lane_ids, -include_lane_details.
///   3. The level of the logger is selected with `-log_level`.

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <gflags/gflags.h>

#include <maliput/common/logger.h>
#include <maliput/plugin/maliput_plugin.h>
#include <maliput/plugin/maliput_plugin_manager.h>
#include <maliput/plugin/road_network_loader.h>
#include <maliput/utilities/generate_string.h>
#include "maliput_gflags.h"

DEFINE_string(plugin_name, "maliput_malidrive", "Id of the RoadNetwork plugin to use.");

// Dragway parameters
DEFINE_string(num_lanes, "2", "The number of lanes.");
DEFINE_string(length, "10", "The length of the dragway in meters.");
DEFINE_string(lane_width, "3.7", "The width of each lane in meters.");
DEFINE_string(shoulder_width, "3.0", "The width of the shoulders in meters. Both shoulders have the same width.");
DEFINE_string(maximum_height, "5.2", "The maximum modelled height above the road surface (meters).");

// Multilane parameters
DEFINE_string(yaml_file, "install/maliput_multilane/share/maliput_multilane/2x2_intersection.yaml",
              "yaml input file defining a multilane road geometry");

// Malidrive parameters
DEFINE_string(opendrive_file, "install/maliput_malidrive/share/maliput_malidrive/resources/odr/LShapeRoad.xodr",
              "XODR file path. Default LShapeRoad.xodr");
DEFINE_string(linear_tolerance, "5e-2", "Linear tolerance used to load the map.");
DEFINE_string(angular_tolerance, "1e-3", "Angular tolerance used to load the map.");
DEFINE_string(scale_length, "1", "Scale length");
DEFINE_string(
    standard_strictness_policy, "permissive",
    "OpenDrive standard strictness, it could be `permissive`, `allow_schema_errors`, `allow_semantic_errors` or "
    "`strict`. Union of policies are also allowed: 'allow_schema_errors|allow_semantic_errors'");

// Gflags to select options for serialization.
DEFINE_bool(include_type_labels, false, "Whether to include type labels in the output string");
DEFINE_bool(include_road_geometry_id, false, "Whether to include road geometry IDs in the output string");
DEFINE_bool(include_junction_ids, false, "Whether to include junction IDs in the output string");
DEFINE_bool(include_segment_ids, false, "Whether to include segment IDs in the output string");
DEFINE_bool(include_lane_ids, false, "Whether to include lane IDs in the output string");
DEFINE_bool(include_lane_details, false, "Whether to include lane details in the output string");

MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

namespace maliput {
namespace integration {
namespace {

int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  const std::map<std::string, std::string> parameters{{"num_lanes", FLAGS_num_lanes},
                                                      {"length", FLAGS_length},
                                                      {"lane_width", FLAGS_lane_width},
                                                      {"shoulder_width", FLAGS_shoulder_width},
                                                      {"maximum_height", FLAGS_maximum_height},
                                                      {"yaml_file", FLAGS_yaml_file},
                                                      {"opendrive_file", FLAGS_opendrive_file},
                                                      {"linear_tolerance", FLAGS_linear_tolerance},
                                                      {"angular_tolerance", FLAGS_angular_tolerance},
                                                      {"scale_length", FLAGS_scale_length},
                                                      {"standard_strictness_policy", FLAGS_standard_strictness_policy}};

  maliput::log()->info("Creating MaliputPluginManager instance...");
  maliput::plugin::MaliputPluginManager manager;
  maliput::log()->info("Plugins loading is completed.");
  const maliput::plugin::MaliputPlugin* maliput_plugin =
      manager.GetPlugin(maliput::plugin::MaliputPlugin::Id(FLAGS_plugin_name));
  if (!maliput_plugin) {
    maliput::log()->error("{} plugin hasn't been found.", FLAGS_plugin_name);
    return 1;
  }
  maliput::log()->info("{} plugin has been found.", FLAGS_plugin_name);
  maliput::log()->info("Plugin id: {}", maliput_plugin->GetId());
  maliput::log()->info(
      "Plugin type: {}",
      (maliput_plugin->GetType() == maliput::plugin::MaliputPluginType::kRoadNetworkLoader ? "RoadNetworkLoader"
                                                                                           : "unknown"));
  // create an instance of the class
  std::unique_ptr<maliput::plugin::RoadNetworkLoader> road_network_loader =
      maliput_plugin->ExecuteSymbol<std::unique_ptr<maliput::plugin::RoadNetworkLoader>>(
          maliput::plugin::RoadNetworkLoader::GetEntryPoint());
  std::unique_ptr<const maliput::api::RoadNetwork> rn = (*road_network_loader)(parameters);

  if (rn == nullptr) {
    maliput::log()->error("RoadNetwork couldn't be loaded correctly.");
    return 1;
  }
  maliput::log()->debug("RoadNetwork loaded successfully.");

  const maliput::utility::GenerateStringOptions options{FLAGS_include_type_labels,  FLAGS_include_road_geometry_id,
                                                        FLAGS_include_junction_ids, FLAGS_include_segment_ids,
                                                        FLAGS_include_lane_ids,     FLAGS_include_lane_details};
  const std::string result = maliput::utility::GenerateString(*(rn->road_geometry()), options);

  std::cout << result << std::endl;
  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
