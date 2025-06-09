// BSD 3-Clause License
//
// Copyright (c) 2024, Woven by Toyota. All rights reserved.
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

/// @file maliput_to_dot.cc
///
/// Builds a dragway, multilane or malidrive road geometry
/// and creates a routing::graph::Graph from it to then serialize to a DOT file representation.
///
/// @note
/// 1. It allows to create an DOT file from different road geometry implementations.
///     The `maliput_backend` flag will determine the backend to be used.
///    - "dragway": The following flags are supported to use in order to create dragway road geometry:
///       -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///    - "multilane": yaml file path must be provided:
///       -yaml_file.
///    - "malidrive": xodr file path must be provided and the tolerance is optional:
///         -xodr_file_path -linear_tolerance.
/// 2. The application possesses flags to modify the DOT file builder:
///      -dot_dir_path, -dot_file_name
/// 3. The log level could be set by: -log_level.

#include <fstream>
#include <limits>
#include <string>

#include <gflags/gflags.h>
#include <maliput/common/filesystem.h>
#include <maliput/common/logger.h>
#include <maliput/common/maliput_abort.h>
#include <maliput/routing/graph/graph.h>
#include <maliput/utility/generate_dot.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

COMMON_PROPERTIES_FLAGS();
MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_OSM_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "dragway", "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");

// Gflags for output files.
DEFINE_string(dot_dir_path, ".", "Directory to contain DOT file.");
DEFINE_string(dot_file_name, "maliput_graph.dot", "Name of the maliput::routing::graph::Graph DOT file.");

namespace maliput {
namespace integration {
namespace {

// Generates an DOT file from a YAML file path or from
// configurable values given as CLI arguments.
int Main(int argc, char* argv[]) {
  static constexpr const char* kUsageMessage = R"*(
Builds a dragway, multilane or malidrive road geometry and creates a routing::graph::Graph from it to then serialize to a DOT file representation.

1. It allows to create an DOT file from different road geometry implementations.
    The `maliput_backend` flag will determine the backend to be used.
   - "dragway": The following flags are supported to use in order to create dragway road geometry:
      -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
   - "multilane": yaml file path must be provided:
      -yaml_file.
   - "malidrive": xodr file path must be provided and the tolerance is optional:
        -xodr_file_path -linear_tolerance.
2. The application possesses flags to modify the DOT file builder:
     -dot_dir_path, -dot_file_name
3. The log level could be set by: -log_level.

Example:

maliput_to_dot --maliput_backend malidrive --xodr_file_path TShapeRoad.xodr
)*";
  gflags::SetUsageMessage(kUsageMessage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  log()->info("Loading road network using ", FLAGS_maliput_backend, " backend implementation...");
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), GetAngularToleranceFlag(),
       FLAGS_build_policy, FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy,
       FLAGS_omit_nondrivable_lanes, FLAGS_rule_registry_file, FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file,
       FLAGS_phase_ring_book_file, FLAGS_intersection_book_file},
      {FLAGS_osm_file, FLAGS_linear_tolerance, FLAGS_max_linear_tolerance,
       maliput::math::Vector2::FromStr(FLAGS_origin), FLAGS_rule_registry_file, FLAGS_road_rule_book_file,
       FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file, FLAGS_intersection_book_file});
  log()->info("RoadNetwork loaded successfully.");

  // Creates the destination directory if it does not already exist.
  common::Path directory;
  directory.set_path(FLAGS_dot_dir_path);
  if (!directory.exists()) {
    common::Filesystem::create_directory_recursive(directory);
  }
  MALIPUT_THROW_UNLESS(directory.exists());

  const std::string dot_file_path = FLAGS_dot_dir_path + "/" + FLAGS_dot_file_name;
  std::ofstream os(dot_file_path, std::ios::binary);

  log()->info("Generating DOT file at ", dot_file_path, " ...");
  utility::GenerateDotStream(routing::graph::BuildGraph(rn->road_geometry()), &os);
  log()->info("Generated DOT file.");

  os.close();

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
