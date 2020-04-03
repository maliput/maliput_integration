/// @file maliput_to_obj.cc
///
/// Builds a dragway road geometry from flags or a multilane road geometry from a file,
/// and render the road surface to a WaveFront OBJ output file.
///
/// Notes:
/// 1 - It allows to create an OBJ file from different road geometry implementations. If a valid
///     filepath of an YAML file is passed, a multilane RoadGeometry will be created. Otherwise,
///     the following arguments will help to carry out a dragway implementation:
///      -num_lanes, -length, -lane_width, -shoulder_width, maximum_height.
/// 2 - The applications possesses flags to modify the OBJ file builder:
///      -obj_dir, -obj_file, -max_grid_unit, -min_grid_resolution, -draw_elevation_bounds, -simplify_mesh_threshold
/// 3 - The level of the logger could be setted by: -log_level.

#include <limits>
#include <string>

#include <gflags/gflags.h>

#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput/utilities/generate_obj.h"

#include "maliput_dragway/road_geometry.h"
#include "maliput_multilane/builder.h"
#include "maliput_multilane/loader.h"

#include "yaml-cpp/yaml.h"

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

// Gflags for output files.
DEFINE_string(obj_dir, ".", "Directory to contain rendered road surface");
DEFINE_string(obj_file, "", "Basename for output Wavefront OBJ and MTL files");

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
DEFINE_string(log_level, "unchanged",
              "sets the spdlog output threshold; possible values are "
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

// Available maliput implementations to load.
enum class MaliputImplementation {
  kDragway,    //< dragway implementation.
  kMultilane,  //< multilane implementation.
  kUnknown     //< Used when none of the implementations could be identified.
};

// Parses a file whose path is `filename` as a YAML and looks for a node called
// "maliput_multilane_builder". If it is found,
// MaliputImplementation::kMultilane is returned. Otherwise,
// MaliputImplementation::kUnknown is returned.
MaliputImplementation GetMaliputImplementationFromFile(const std::string& filename) {
  const YAML::Node yaml_file = YAML::LoadFile(filename);
  MALIPUT_DEMAND(yaml_file.IsMap());
  return yaml_file["maliput_multilane_builder"] ? MaliputImplementation::kMultilane : MaliputImplementation::kUnknown;
}

// Decides which type of road geometry implementation the application should select.
// If `filename` is empty it returns MaliputImplementation:kDragway.
// If `filename` is not empty it will load the file and check which implementation it refers to.
MaliputImplementation GetMaliputImplementation(const std::string& filename) {
  return filename.empty() ? MaliputImplementation::kDragway : GetMaliputImplementationFromFile(filename);
}

// Generates an OBJ file from a YAML file path or from
// configurable values given as CLI arguments.
int main(int argc, char* argv[]) {
  maliput::log()->debug("main()");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  maliput::common::set_log_level(FLAGS_log_level);

  if (FLAGS_obj_file.empty()) {
    maliput::log()->critical("No output file specified.");
    return 1;
  }

  maliput::log()->info("Loading road geometry...");
  std::unique_ptr<const maliput::api::RoadGeometry> rg{};
  switch (GetMaliputImplementation(FLAGS_yaml_file)) {
    case MaliputImplementation::kDragway: {
      rg = std::make_unique<dragway::RoadGeometry>(
          api::RoadGeometryId{"Dragway with " + std::to_string(FLAGS_num_lanes) + " lanes."}, FLAGS_num_lanes,
          FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height,
          std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon());
      maliput::log()->info("Loaded a dragway road geometry.");
      break;
    }
    case MaliputImplementation::kMultilane: {
      rg = maliput::multilane::LoadFile(maliput::multilane::BuilderFactory(), FLAGS_yaml_file);
      maliput::log()->info("Loaded a multilane road geometry.");
      break;
    }
    case MaliputImplementation::kUnknown: {
      maliput::log()->error("Unknown map.");
      return 1;
    }
  }

  utility::ObjFeatures features;
  features.max_grid_unit = FLAGS_max_grid_unit;
  features.min_grid_resolution = FLAGS_min_grid_resolution;
  features.draw_elevation_bounds = FLAGS_draw_elevation_bounds;
  features.simplify_mesh_threshold = FLAGS_simplify_mesh_threshold;
  maliput::log()->info("Generating OBJ.");
  GenerateObjFile(rg.get(), FLAGS_obj_dir, FLAGS_obj_file, features);

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::main(argc, argv); }
