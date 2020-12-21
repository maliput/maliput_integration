#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <gflags/gflags.h>

#include "maliput/common/logger.h"
#include "maliput/plugin/load_road_network.h"
#include "maliput/utilities/generate_string.h"

DEFINE_string(lib_name, "libmaliput_dragway_road_network.so", "Name of the .so file to be loaded.");

// Dragway parameters
DEFINE_string(num_lanes, "2", "The number of lanes.");
DEFINE_string(length, "10", "The length of the dragway in meters.");
DEFINE_string(lane_width, "3.7", "The width of each lane in meters.");
DEFINE_string(shoulder_width, "3.0", "The width of the shoulders in meters. Both shoulders have the same width.");
DEFINE_string(maximum_height, "5.2", "The maximum modelled height above the road surface (meters).");

// Gflags to select options for serialization.
DEFINE_bool(include_type_labels, false, "Whether to include type labels in the output string");
DEFINE_bool(include_road_geometry_id, false, "Whether to include road geometry IDs in the output string");
DEFINE_bool(include_junction_ids, false, "Whether to include junction IDs in the output string");
DEFINE_bool(include_segment_ids, false, "Whether to include segment IDs in the output string");
DEFINE_bool(include_lane_ids, false, "Whether to include lane IDs in the output string");
DEFINE_bool(include_lane_details, false, "Whether to include lane details in the output string");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  const std::map<std::string, std::string> parameters{
    {"num_lanes", FLAGS_num_lanes},
    {"length", FLAGS_length},
    {"lane_width", FLAGS_lane_width},
    {"shoulder_width", FLAGS_shoulder_width},
    {"maximum_height", FLAGS_maximum_height}
  };
  maliput::plugin::LoadRoadNetworkPlugin loader{FLAGS_lib_name, parameters};

  // create an instance of the class
  std::unique_ptr<const maliput::api::RoadNetwork> rn = loader.GetRoadNetwork();

  maliput::log()->debug("RoadNetwork loaded successfully.");

  const maliput::utility::GenerateStringOptions options{FLAGS_include_type_labels,  FLAGS_include_road_geometry_id,
                                                        FLAGS_include_junction_ids, FLAGS_include_segment_ids,
                                                        FLAGS_include_lane_ids,     FLAGS_include_lane_details};
  const std::string result = maliput::utility::GenerateString(*(rn->road_geometry()), options);

  std::cout << result << std::endl;
  return 0;
}
