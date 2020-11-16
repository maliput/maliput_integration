#include "integration/tools.h"

#include <map>

#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput_dragway/road_geometry.h"

#include "maliput_malidrive/base/inertial_to_lane_mapping_config.h"
#include "maliput_malidrive/builder/road_geometry_builder.h"

#include "maliput_multilane/builder.h"
#include "maliput_multilane/loader.h"

#include "yaml-cpp/yaml.h"

namespace maliput {
namespace integration {
namespace {

// Holds the conversions from MaliputImplementation to std::string.
const std::map<MaliputImplementation, std::string> maliput_impl_to_string{
    {MaliputImplementation::kDragway, "dragway"},
    {MaliputImplementation::kMalidrive, "malidrive"},
    {MaliputImplementation::kMultilane, "multilane"},
};

// Holds the conversions from std::string to MaliputImplementation.
const std::map<std::string, MaliputImplementation> string_to_maliput_impl{
    {"dragway", MaliputImplementation::kDragway},
    {"malidrive", MaliputImplementation::kMalidrive},
    {"multilane", MaliputImplementation::kMultilane},
};

}  // namespace

std::string MaliputImplementationToString(MaliputImplementation maliput_impl) {
  return maliput_impl_to_string.at(maliput_impl);
}

MaliputImplementation StringToMaliputImplementation(const std::string& maliput_impl) {
  MALIPUT_DEMAND(string_to_maliput_impl.find(maliput_impl) != string_to_maliput_impl.end());
  return string_to_maliput_impl.at(maliput_impl);
}

std::unique_ptr<const api::RoadGeometry> CreateDragwayRoadGeometry(const DragwayBuildProperties& build_properties) {
  maliput::log()->debug("Building dragway RoadGeometry.");
  return std::make_unique<dragway::RoadGeometry>(
      api::RoadGeometryId{"Dragway with " + std::to_string(build_properties.num_lanes) + " lanes."},
      build_properties.num_lanes, build_properties.length, build_properties.lane_width, build_properties.shoulder_width,
      build_properties.maximum_height, std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon());
}

std::unique_ptr<const api::RoadGeometry> CreateMultilaneRoadGeometry(const MultilaneBuildProperties& build_properties) {
  maliput::log()->debug("Building multilane RoadGeometry.");
  if (build_properties.yaml_file.empty()) {
    MALIPUT_ABORT_MESSAGE("yaml_file cannot be empty.");
  }
  return maliput::multilane::LoadFile(maliput::multilane::BuilderFactory(), build_properties.yaml_file);
}

std::unique_ptr<const api::RoadGeometry> CreateMalidriveRoadGeometry(const MalidriveBuildProperties& build_properties) {
  const malidrive::builder::RoadGeometryConfiguration road_geometry_configuration{
      maliput::api::RoadGeometryId("malidrive_rg"),
      build_properties.xodr_file_path,
      build_properties.linear_tolerance,
      malidrive::constants::kAngularTolerance,
      malidrive::constants::kScaleLength,
      malidrive::InertialToLaneMappingConfig(malidrive::constants::kExplorationRadius,
                                             malidrive::constants::kNumIterations)};

  if (!road_geometry_configuration.opendrive_file.has_value()) {
    MALIPUT_ABORT_MESSAGE("opendrive_file cannot be empty.");
  }
  std::unique_ptr<malidrive::builder::RoadCurveFactoryBase> road_curve_factory =
      std::make_unique<malidrive::builder::RoadCurveFactory>(road_geometry_configuration.linear_tolerance,
                                                             road_geometry_configuration.scale_length,
                                                             road_geometry_configuration.angular_tolerance);
  return malidrive::builder::RoadGeometryBuilder(
      malidrive::xodr::LoadDataBaseFromFile(road_geometry_configuration.opendrive_file.value(),
                                            road_geometry_configuration.linear_tolerance),
      road_geometry_configuration, std::move(road_curve_factory))();
}

}  // namespace integration
}  // namespace maliput
