#include <iostream>
#include <memory>
#include <string>

#include <gflags/gflags.h>

#include "maliput/common/logger.h"
#include "maliput/plugin/load_road_network.h"
#include "maliput/utilities/generate_string.h"

DEFINE_string(lib_name, "libmaliput_dragway_road_network.so", "Name of the .so file to be loaded.");

// Gflags to select options for serialization.
DEFINE_bool(include_type_labels, false, "Whether to include type labels in the output string");
DEFINE_bool(include_road_geometry_id, false, "Whether to include road geometry IDs in the output string");
DEFINE_bool(include_junction_ids, false, "Whether to include junction IDs in the output string");
DEFINE_bool(include_segment_ids, false, "Whether to include segment IDs in the output string");
DEFINE_bool(include_lane_ids, false, "Whether to include lane IDs in the output string");
DEFINE_bool(include_lane_details, false, "Whether to include lane details in the output string");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  maliput::plugin::LoadRoadNetworkPlugin loader(FLAGS_lib_name);

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
