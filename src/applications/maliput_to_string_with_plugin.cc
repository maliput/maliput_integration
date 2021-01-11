#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <gflags/gflags.h>

#include "maliput/common/logger.h"
#include "maliput/plugin/maliput_plugin.h"
#include "maliput/plugin/maliput_plugin_manager.h"
#include "maliput/plugin/road_network_loader.h"
#include "maliput/utilities/generate_string.h"
#include "maliput_gflags.h"

DEFINE_string(plugin_name, "maliput_malidrive", "Id of the RoadNetwork plugin to use.");

// Dragway parameters
DEFINE_string(num_lanes, "2", "The number of lanes.");
DEFINE_string(length, "10", "The length of the dragway in meters.");
DEFINE_string(lane_width, "3.7", "The width of each lane in meters.");
DEFINE_string(shoulder_width, "3.0", "The width of the shoulders in meters. Both shoulders have the same width.");
DEFINE_string(maximum_height, "5.2", "The maximum modelled height above the road surface (meters).");

// Malidrive parameters
DEFINE_string(opendrive_file, "install/maliput_malidrive/share/maliput_malidrive/resources/odr/LShapeRoad.xodr",
              "XODR file path. Default LShapeRoad.xodr");
DEFINE_string(linear_tolerance, "5e-2", "Linear tolerance used to load the map.");
DEFINE_string(angular_tolerance, "1e-3", "Angular tolerance used to load the map.");
DEFINE_string(scale_map, "1", "Scale map");

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
                                                      {"opendrive_file", FLAGS_opendrive_file},
                                                      {"linear_tolerance", FLAGS_linear_tolerance},
                                                      {"angular_tolerance", FLAGS_angular_tolerance},
                                                      {"scale_map", FLAGS_scale_map}};

  maliput::log()->info("Creating MaliputPluginManager instance...");
  maliput::plugin::MaliputPluginManager manager;
  maliput::log()->info("Plugins loading is completed.");
  const maliput::plugin::MaliputPlugin* maliput_plugin =
      manager.GetPlugin(maliput::plugin::MaliputPlugin::Id(FLAGS_plugin_name));
  if (!maliput_plugin) {
    maliput::log()->error("{} plugin hasn't been found.", FLAGS_plugin_name);
    return 1;
  } else {
    maliput::log()->info("{} plugin has been found.", FLAGS_plugin_name);
    maliput::log()->info("Plugin id: {}", maliput_plugin->GetId());
    maliput::log()->info(
        "Plugin type: {}",
        (maliput_plugin->GetType() == maliput::plugin::MaliputPluginType::kRoadNetworkLoader ? "RoadNetworkLoader"
                                                                                             : "unknown"));
  }
  // create an instance of the class
  std::unique_ptr<maliput::plugin::RoadNetworkLoader> road_network_loader =
      maliput_plugin->ExecuteSymbol<std::unique_ptr<maliput::plugin::RoadNetworkLoader>>(
          maliput::plugin::RoadNetworkLoader::GetEntryPoint());
  std::unique_ptr<const maliput::api::RoadNetwork> rn = (*road_network_loader)(parameters);

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
