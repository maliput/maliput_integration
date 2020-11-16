/// @file maliput_to_obj.cc
///
/// Builds a dragway road geometry from flags or a multilane road geometry from a file,
/// and render the road surface to a WaveFront OBJ output file.
///
/// Notes:
/// 1 - It allows to create an OBJ file from different road geometry implementations. If a valid
///     filepath of an YAML file is passed, a multilane RoadGeometry will be created. Otherwise,
///     the following arguments will help to carry out a dragway implementation:
///      -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
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
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

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
  GenerateObjFile(rg.get(), FLAGS_dirpath, FLAGS_file_name_root, features);

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
