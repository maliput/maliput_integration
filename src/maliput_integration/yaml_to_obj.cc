/// @file yaml_to_obj.cc
///
/// Take a yaml file as input, build the resulting multilane road geometry, and
/// render the road surface to a WaveFront OBJ output file.
#include <string>

#include <gflags/gflags.h>

#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput/utilities/generate_obj.h"

#include "multilane/builder.h"
#include "multilane/loader.h"

#include "yaml-cpp/yaml.h"

DEFINE_string(yaml_file, "", "yaml input file defining a multilane road geometry");
DEFINE_string(obj_dir, ".", "Directory to contain rendered road surface");
DEFINE_string(obj_file, "", "Basename for output Wavefront OBJ and MTL files");
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
DEFINE_string(spdlog_level, "unchanged",
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
namespace utility {
namespace {

// Available maliput implementations to load.
enum class MaliputImplementation {
  kMultilane,  //< multilane implementation.
  kUnknown     //< Used when none of the implementations could be identified.
};

// Parses a file whose path is `filename` as a YAML and looks for a node called
// "maliput_multilane_builder". If it is found,
// MaliputImplementation::kMultilane is returned. Otherwise,
// MaliputImplementation::kUnknown is returned.
MaliputImplementation GetMaliputImplementation(const std::string& filename) {
  const YAML::Node yaml_file = YAML::LoadFile(filename);
  MALIPUT_DEMAND(yaml_file.IsMap());
  if (yaml_file["maliput_multilane_builder"]) {
    return MaliputImplementation::kMultilane;
  }
  return MaliputImplementation::kUnknown;
}

// Generates an OBJ file from a YAML file path given as CLI argument.
int main(int argc, char* argv[]) {
  maliput::log()->debug("main()");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  maliput::common::set_log_level(FLAGS_spdlog_level);

  if (FLAGS_yaml_file.empty()) {
    maliput::log()->critical("No input file specified.");
    return 1;
  }
  if (FLAGS_obj_file.empty()) {
    maliput::log()->critical("No output file specified.");
    return 1;
  }

  maliput::log()->info("Loading road geometry...");
  std::unique_ptr<const maliput::api::RoadGeometry> rg{};
  switch (GetMaliputImplementation(FLAGS_yaml_file)) {
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
}  // namespace utility
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::utility::main(argc, argv); }
