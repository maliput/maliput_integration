/// @file maliput_to_string.cc
///
/// Builds an api::RoadGeometry and lists its entities. Possible backends are `dragway` and `multilane`.
///
/// Notes:
///   1- Allows to load a `dragway` or a `multilane` api::RoadGeometry. When a valid file path to a YAML document
///       is passed, a `multilane` api::RoadGeometry is built. Otherwise, the following arguments will help to
///       carry out a dragway implementation:
///      -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///   2 - The applications possesses flags to modify the output serialization:
///      -include_type_labels, -include_road_geometry_id, -include_junction_ids,
///      -include_segment_ids, -include_lane_ids, -include_lane_details.
///   3- The level of the logger is selected with `-log_level`.

#include <iostream>
#include <memory>
#include <string>

#include <gflags/gflags.h>

#include "integration/tools.h"

#include "maliput/common/logger.h"
#include "maliput/utilities/generate_string.h"

// Gflags for road geometry loaded from file/
DEFINE_string(yaml_file, "", "yaml input file defining a multilane road geometry");

// Gflags for implementing a dragway road geometry manually.
DEFINE_int32(num_lanes, 2, "The number of lanes.");
DEFINE_double(length, 10, "The length of the dragway in meters.");
// By default, each lane is 3.7m (12 feet) wide, which is the standard used by
// the U.S. interstate highway system.
DEFINE_double(lane_width, 3.7, "The width of each lane in meters.");
// By default, the shoulder width is 3 m (10 feet) wide, which is the standard
// used by the U.S. interstate highway system.
DEFINE_double(shoulder_width, 3.0,
              "The width of the shoulders in meters. Both shoulders have the same "
              "width.");
DEFINE_double(maximum_height, 5.2, "The maximum modelled height above the road surface (meters).");

// Gflags to select options for serialization.
DEFINE_bool(include_type_labels, false, "Whether to include type labels in the output string");
DEFINE_bool(include_road_geometry_id, false, "Whether to include road geometry IDs in the output string");
DEFINE_bool(include_junction_ids, false, "Whether to include junction IDs in the output string");
DEFINE_bool(include_segment_ids, false, "Whether to include segment IDs in the output string");
DEFINE_bool(include_lane_ids, false, "Whether to include lane IDs in the output string");
DEFINE_bool(include_lane_details, false, "Whether to include lane details in the output string");
// Gflag to set the log output threshold.
DEFINE_string(log_level, "unchanged",
              "sets the log output threshold; possible values are "
              "'unchanged', "
              "'trace', "
              "'debug', "
              "'info', "
              "'warn', "
              "'err', "
              "'critical', "
              "'off'");

namespace maliput {
namespace integration {
namespace {

int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  maliput::common::set_log_level(FLAGS_log_level);

  log()->debug("Loading road geometry...");
  const std::optional<std::string> yaml_file =
      !FLAGS_yaml_file.empty() ? std::make_optional<std::string>(FLAGS_yaml_file) : std::nullopt;
  const std::optional<DragwayBuildProperties> dragway_build_properties =
      !FLAGS_yaml_file.empty()
          ? std::nullopt
          : std::make_optional<DragwayBuildProperties>(DragwayBuildProperties{
                FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_lane_width, FLAGS_lane_width});
  std::unique_ptr<const api::RoadGeometry> rg = CreateRoadGeometryFrom(yaml_file, dragway_build_properties);
  if (rg == nullptr) {
    log()->error("Error loading RoadGeometry");
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
