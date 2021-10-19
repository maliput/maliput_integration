/// @file maliput_to_string.cc
///
/// Builds an api::RoadGeometry and lists its entities. Possible backends are `dragway`, `multilane` and `malidrive`.
///
/// @note
///   1. Allows to load a road geometry from different road geometry implementations.
///       The `maliput_backend` flag will determine the backend to be used.
///      - "dragway": The following flags are supported to use in order to create dragway road geometry:
///           -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///      - "multilane": yaml file path must be provided:
///           -yaml_file.
///      - "malidrive": xodr file path must be provided and the tolerance is optional:
///           -xodr_file_path -linear_tolerance.
///   2. The applications possesses flags to modify the output serialization:
///      -include_type_labels, -include_road_geometry_id, -include_junction_ids,
///      -include_segment_ids, -include_lane_ids, -include_lane_details.
///   3. The level of the logger is selected with `-log_level`.

#include <iostream>
#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <maliput/common/logger.h>
#include <maliput/utilities/generate_string.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "malidrive",
              "Whether to use <dragway>, <multilane> or <malidrive>. Default is malidrive.");
DEFINE_bool(check_invariants, false, "Whether to enable maliput invariants verification.");
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

  log()->info("Loading road network using {} backend implementation...", FLAGS_maliput_backend);
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), FLAGS_build_policy,
       FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy, FLAGS_omit_nondrivable_lanes,
       FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file});
  log()->info("RoadNetwork loaded successfully.");
  if (FLAGS_check_invariants) {
    log()->info("Checking invariants...");
    const auto violations = rn->road_geometry()->CheckInvariants();
    violations.empty() ? log()->info("No invariant violations were found.")
                       : log()->warn("{} invariant violations were found: ", violations.size());
    for (const auto& v : violations) {
      log()->warn(v);
    }
  }

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
