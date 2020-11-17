/// @file maliput_to_obj.cc
///
/// Builds a dragway, multilane or malidrive road geometry
/// and render the road surface to a WaveFront OBJ output file.
///
/// Notes:
/// 1 - It allows to create an OBJ file from different road geometry implementations.
///     The `maliput_backend` flag will determine the backend to be used.
///    A - "dragway": The following flags are supported to use in order to create dragway road geometry:
///         -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///    B - "multilane": yaml file path must be provided:
///         -yaml_file.
///    C - "malidrive": xodr file path must be provided and the tolerance is optional:
///         -xodr_file_path -linear_tolerance.
/// 2 - The applications possesses flags to modify the OBJ file builder:
///      -obj_dir, -obj_file, -max_grid_unit, -min_grid_resolution, -draw_elevation_bounds, -simplify_mesh_threshold
/// 3 - The level of the logger could be setted by: -log_level.

#include <limits>
#include <string>

#include <gflags/gflags.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

#include "maliput/common/filesystem.h"
#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"
#include "maliput/utilities/generate_obj.h"

#include "yaml-cpp/yaml.h"

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "dragway", "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");

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

namespace maliput {
namespace integration {
namespace {

// Generates an OBJ file from a YAML file path or from
// configurable values given as CLI arguments.
int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  log()->debug("Loading road geometry using {} backend implementation...", FLAGS_maliput_backend);
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  std::unique_ptr<const api::RoadNetwork> rn;
  std::unique_ptr<const api::RoadGeometry> rg;
  switch (maliput_implementation) {
    case MaliputImplementation::kDragway:
      rg = CreateDragwayRoadGeometry(
          {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height});
      break;
    case MaliputImplementation::kMultilane:
      rn = CreateMultilaneRoadNetwork({FLAGS_yaml_file});
      break;
    case MaliputImplementation::kMalidrive:
      rn = CreateMalidriveRoadNetwork({FLAGS_xodr_file_path, FLAGS_linear_tolerance});
      break;
    default:
      log()->error("Error loading RoadGeometry. Unknown implementation.");
      return 1;
  }
  log()->debug("RoadGeometry loaded successfully");

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

  const common::Path my_path = common::Filesystem::get_cwd();
  FLAGS_dirpath == "." ? log()->info("OBJ files location: {}.", my_path.get_path())
                       : log()->info("OBJ files location: {}.", FLAGS_dirpath);

  log()->debug("Generating OBJ.");
  GenerateObjFile(maliput_implementation == MaliputImplementation::kDragway ? rg.get() : rn->road_geometry(),
                  FLAGS_dirpath, FLAGS_file_name_root, features);

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
