/// @file maliput_to_urdf.cc
///
/// Builds a dragway, multilane or malidrive road geometry,
/// and outputs a URDF model of it.
///
/// Notes:
/// 1 - It allows to create URDF files from different road geometry implementations.
///     The `maliput_backend` flag will determine the backend to be used.
///    A - "dragway": The following flags are supported to use in order to create dragway road geometry:
///         -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///    B - "multilane": yaml file path must be provided:
///         -yaml_file.
///    C - "malidrive": xodr file path must be provided and the tolerance is optional:
///         -xodr_file_path -linear_tolerance.
/// 2 - The level of the logger could be setted by: -log_level.

#include <limits>
#include <string>

#include <gflags/gflags.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

#include "maliput/common/filesystem.h"
#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"
#include "maliput/utilities/generate_urdf.h"

#include "yaml-cpp/yaml.h"

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "dragway", "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");

// Gflags for output files.
DEFINE_string(dirpath, ".",
              "The path to where the URDF and OBJ files should be saved. If this path "
              " does not exist, it is created.");
DEFINE_string(file_name_root, "maliput_to_urdf",
              "The root name of the files to create. For example, if the value of this "
              "parameter is \"foo\", the following files will be created: \"foo.urdf\", "
              "\"foo.obj\", and \"foo.mtl\". These files will be placed in the path "
              "specified by parameter 'dirpath'.");

namespace maliput {
namespace integration {
namespace {

int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  log()->debug("Loading road geometry using {} backend implementation...", FLAGS_maliput_backend);
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  std::unique_ptr<const api::RoadNetwork> malidrive_rn;
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
      malidrive_rn = CreateMalidriveRoadNetwork({FLAGS_xodr_file_path, FLAGS_linear_tolerance});
      break;
    default:
      log()->error("Error loading RoadGeometry. Unknown implementation.");
      return 1;
  }
  log()->debug("RoadGeometry loaded successfully");

  utility::ObjFeatures features;

  // Creates the destination directory if it does not already exist.
  common::Path directory;
  directory.set_path(FLAGS_dirpath);
  if (!directory.exists()) {
    common::Filesystem::create_directory_recursive(directory);
  }
  MALIPUT_THROW_UNLESS(directory.exists());

  const common::Path my_path = common::Filesystem::get_cwd();
  FLAGS_dirpath == "." ? log()->info("URDF files location: {}.", my_path.get_path())
                       : log()->info("URDF files location: {}.", FLAGS_dirpath);

  log()->debug("Generating URDF files.");
  utility::GenerateUrdfFile(rg.get(), directory.get_path(), FLAGS_file_name_root, features);
  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
