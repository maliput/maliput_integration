/// @file maliput_to_string.cc
///
/// Builds an api::RoadGeometry and lists its entities. Possible backends are `dragway`, `multilane` and `malidrive`.
///
/// Notes:
///   1 - Allows to load a road geometry from different road geometry implementations.
///       The `maliput_backend` flag will determine the backend to be used.
///      A - "dragway": The following flags are supported to use in order to create dragway road geometry:
///           -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///      B - "multilane": yaml file path must be provided:
///           -yaml_file.
///      C - "malidrive": xodr file path must be provided and the tolerance is optional:
///           -xodr_file_path -linear_tolerance.
///   2 - The applications possesses flags to modify the output serialization:
///      -include_type_labels, -include_road_geometry_id, -include_junction_ids,
///      -include_segment_ids, -include_lane_ids, -include_lane_details.
///   3- The level of the logger is selected with `-log_level`.

#include <iostream>
#include <memory>
#include <string>

#include <gflags/gflags.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

#include "maliput/common/logger.h"
#include "maliput/utilities/generate_string.h"

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "dragway", "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");

// Gflags to select options for serialization.
DEFINE_bool(include_type_labels, false, "Whether to include type labels in the output string");
DEFINE_bool(include_road_geometry_id, false, "Whether to include road geometry IDs in the output string");
DEFINE_bool(include_junction_ids, false, "Whether to include junction IDs in the output string");
DEFINE_bool(include_segment_ids, false, "Whether to include segment IDs in the output string");
DEFINE_bool(include_lane_ids, false, "Whether to include lane IDs in the output string");
DEFINE_bool(include_lane_details, false, "Whether to include lane details in the output string");

namespace maliput {
namespace integration {
namespace {

int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  maliput::common::set_log_level(FLAGS_log_level);

  log()->debug("Loading road geometry using {} backend implementation...", FLAGS_maliput_backend);
  MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  std::unique_ptr<const api::RoadGeometry> rg;
  switch (maliput_implementation) {
    case MaliputImplementation::kDragway:
      rg = CreateDragwayRoadGeometry(
          {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height});
      break;
    case MaliputImplementation::kMultilane:
      rg = CreateMultilaneRoadGeometry({FLAGS_yaml_file});
      break;
    case MaliputImplementation::kMalidrive:
      rg = CreateMalidriveRoadGeometry({FLAGS_xodr_file_path, FLAGS_linear_tolerance});
      break;
    default:
      log()->error("Error loading RoadGeometry. Unknown implementation.");
      return 1;
  }
  log()->debug("RoadGeometry loaded successfully");

  const maliput::utility::GenerateStringOptions options{FLAGS_include_type_labels,  FLAGS_include_road_geometry_id,
                                                        FLAGS_include_junction_ids, FLAGS_include_segment_ids,
                                                        FLAGS_include_lane_ids,     FLAGS_include_lane_details};
  const std::string result = maliput::utility::GenerateString(*rg, options);

  std::cout << result << std::endl;
  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
