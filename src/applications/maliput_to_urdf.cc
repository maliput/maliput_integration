/// @file maliput_to_urdf.cc
///
/// Builds a dragway road geometry from flags or a multilane road geometry from a file,
/// and outputs a URDF model of it.
///
/// Notes:
/// 1 - It allows to create URDF files from different road geometry implementations. If a valid
///     filepath of an YAML file is passed, a multilane RoadGeometry will be created. Otherwise,
///     the following arguments will help to carry out a dragway implementation:
///      -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
/// 2 - The level of the logger could be setted by: -log_level.

#include <limits>
#include <string>

#include <gflags/gflags.h>

#include "integration/tools.h"

#include "maliput/common/filesystem.h"
#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"
#include "maliput/utilities/generate_urdf.h"

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
DEFINE_string(dirpath, ".",
              "The path to where the URDF and OBJ files should be saved. If this path "
              " does not exist, it is created.");
DEFINE_string(file_name_root, "maliput_to_urdf",
              "The root name of the files to create. For example, if the value of this "
              "parameter is \"foo\", the following files will be created: \"foo.urdf\", "
              "\"foo.obj\", and \"foo.mtl\". These files will be placed in the path "
              "specified by parameter 'dirpath'.");
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
