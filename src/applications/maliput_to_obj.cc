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

/// @file maliput_to_obj.cc
///
/// Builds a dragway, multilane or malidrive road geometry
/// and render the road surface to a WaveFront OBJ output file.
///
/// @note
/// 1. It allows to create an OBJ file from different road geometry implementations.
///     The `maliput_backend` flag will determine the backend to be used.
///    - "dragway": The following flags are supported to use in order to create dragway road geometry:
///       -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///    - "multilane": yaml file path must be provided:
///       -yaml_file.
///    - "malidrive": xodr file path must be provided and the tolerance is optional:
///         -xodr_file_path -linear_tolerance.
/// 2. The application possesses flags to modify the OBJ file builder:
///      -obj_dir, -obj_file, -max_grid_unit, -min_grid_resolution, -draw_elevation_bounds, -simplify_mesh_threshold
/// 3. An urdf file can also be created by passing -urdf flag.
/// 4. The log level could be set by: -log_level.

#include <chrono>
#include <limits>
#include <string>

#include <gflags/gflags.h>
#include <maliput/common/filesystem.h>
#include <maliput/common/logger.h>
#include <maliput/common/maliput_abort.h>
#include <maliput/utility/generate_obj.h>
#include <maliput/utility/generate_urdf.h>
#include <yaml-cpp/yaml.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

COMMON_PROPERTIES_FLAGS();
MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_OSM_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "dragway", "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");

// Gflag to enable .urdf file creation.
DEFINE_bool(urdf, false, "Enable URDF file creation.");

// Gflags for output files.
DEFINE_string(dirpath, ".", "Directory to contain rendered road surface");
DEFINE_string(file_name_root, "maliput_to_obj", "Basename for output Wavefront OBJ and MTL files");

// Gflags for OBJ generation configuration.
DEFINE_double(max_grid_unit, maliput::utility::ObjFeatures().max_grid_unit,
              "Maximum size of a grid unit in the rendered mesh covering the "
              "road surface");
DEFINE_double(min_grid_resolution, maliput::utility::ObjFeatures().min_grid_resolution,
              "Minimum number of grid-units in either lateral or longitudinal "
              "direction in the rendered mesh covering the road surface");
DEFINE_bool(draw_elevation_bounds, maliput::utility::ObjFeatures().draw_elevation_bounds,
            "Whether to draw the elevation bounds");
DEFINE_double(simplify_mesh_threshold, maliput::utility::ObjFeatures().simplify_mesh_threshold,
              "Optional tolerance for mesh simplification, in meters. Make it "
              "equal to the road linear tolerance to get a mesh size reduction "
              "while keeping geometrical fidelity.");
DEFINE_bool(draw_arrows, maliput::utility::ObjFeatures().draw_arrows,
            "Whether to draw arrows for indicating the direction of the road ");
DEFINE_bool(draw_branch_points, maliput::utility::ObjFeatures().draw_branch_points,
            "Whether to draw the branch points of the road");
DEFINE_bool(draw_stripes, maliput::utility::ObjFeatures().draw_stripes,
            "Whether to draw stripes along boundaries of each lane");
DEFINE_bool(draw_lane_haze, maliput::utility::ObjFeatures().draw_lane_haze,
            "Whether to draw the highlighting swath with boundaries of each lane");
DEFINE_bool(off_grid_mesh_generation, maliput::utility::ObjFeatures().off_grid_mesh_generation,
            "Whether to reduce the amount of vertices from the road by creating "
            "quads big enough which don't violate some tolerance. This could "
            "affect the accuracy of curved roads.");

namespace maliput {
namespace integration {

// Returns a string with the usage message.
std::string GetUsageMessage() {
  std::stringstream ss;
  ss << "Create a OBJ (WAVEFRONT) file from a maliput road network." << std::endl << std::endl;
  ss << "  maliput_to_obj <OPTIONS> " << std::endl << std::endl;
  ss << "  Examples of use: " << std::endl;
  ss << "    $ maliput_to_obj --maliput_backend=malidrive --xodr_file_path=Town07.xodr --build_policy=parallel "
        "--linear_tolerance=0.05 --max_linear_tolerance=0.05 --log_level=info --dirpath=src/maliput_malidrive/obj/ "
        "--file_name_root=TShapeRoadOBJ --off_grid_mesh_generation=True --draw_elevation_bounds=False"
     << std::endl;

  return ss.str();
}
namespace {
// Generates an OBJ file from a YAML file path or from
// configurable values given as CLI arguments.
int Main(int argc, char* argv[]) {
  gflags::SetUsageMessage(GetUsageMessage());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  log()->info("Loading road network using ", FLAGS_maliput_backend, " backend implementation...");
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  const auto load_time_now = std::chrono::system_clock::now();
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), GetAngularToleranceFlag(),
       FLAGS_build_policy, FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy,
       FLAGS_omit_nondrivable_lanes, FLAGS_integrator_accuracy_multiplier, FLAGS_rule_registry_file,
       FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file},
      {FLAGS_osm_file, FLAGS_linear_tolerance, FLAGS_max_linear_tolerance,
       maliput::math::Vector2::FromStr(FLAGS_origin), FLAGS_rule_registry_file, FLAGS_road_rule_book_file,
       FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file, FLAGS_intersection_book_file});
  log()->info(
      "RoadNetwork loaded successfully in ",
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - load_time_now).count(),
      " ms.");

  // Creates the destination directory if it does not already exist.
  common::Path directory;
  directory.set_path(FLAGS_dirpath);
  if (!directory.exists()) {
    common::Filesystem::create_directory_recursive(directory);
  }
  MALIPUT_THROW_UNLESS(directory.exists());

  utility::ObjFeatures features;
  features.max_grid_unit = FLAGS_max_grid_unit;
  features.min_grid_resolution = FLAGS_min_grid_resolution;
  features.draw_elevation_bounds = FLAGS_draw_elevation_bounds;
  features.simplify_mesh_threshold = FLAGS_simplify_mesh_threshold;
  features.draw_arrows = FLAGS_draw_arrows;
  features.draw_branch_points = FLAGS_draw_branch_points;
  features.draw_stripes = FLAGS_draw_stripes;
  features.draw_lane_haze = FLAGS_draw_lane_haze;
  features.off_grid_mesh_generation = FLAGS_off_grid_mesh_generation;

  const common::Path my_path = common::Filesystem::get_cwd();
  const std::string urdf = FLAGS_urdf ? "/URDF" : "";
  FLAGS_dirpath == "." ? log()->info("OBJ", urdf, " files location: ", my_path.get_path(), ".")
                       : log()->info("OBJ", urdf, " files location: ", FLAGS_dirpath, ".");

  log()->info("Generating OBJ", urdf, " ...");
  const auto now = std::chrono::system_clock::now();
  FLAGS_urdf ? GenerateUrdfFile(rn->road_geometry(), FLAGS_dirpath, FLAGS_file_name_root, features)
             : GenerateObjFile(rn->road_geometry(), FLAGS_dirpath, FLAGS_file_name_root, features);
  const auto elapsed = std::chrono::system_clock::now() - now;
  log()->info("OBJ", urdf, " creation has finished in ",
              std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), " ms.");
  log()->info("OBJ", urdf, " files location: ", FLAGS_dirpath, ".");

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
