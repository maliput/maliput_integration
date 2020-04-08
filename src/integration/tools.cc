#include "integration/tools.h"

#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput_dragway/road_geometry.h"

#include "maliput_multilane/builder.h"
#include "maliput_multilane/loader.h"

#include "yaml-cpp/yaml.h"

namespace maliput {
namespace integration {
namespace {

// Available maliput implementations.
enum class MaliputImplementation {
  kDragway,    //< dragway implementation.
  kMultilane,  //< multilane implementation.
  kUnknown     //< Used when none of the implementations could be identified.
};

// Indicates whether `filename` correspond with a multilane description.
//
// @param filename Is the path to the filename.
// @returns True when the file describes a multilane::RoadGeometry.
// @throw maliput::common::assertion_error When YAML file is not well-constructed.
bool IsMultilaneDescription(const std::string& filename) {
  const YAML::Node yaml_file = YAML::LoadFile(filename);
  MALIPUT_DEMAND(yaml_file.IsMap());
  return yaml_file["maliput_multilane_builder"] ? true : false;
}

// Decides which type of road geometry implementation should be choosen.
// If only `filename` has a value it will return MaliputImplementation::kMultilane.
// If only `dragway_build_properties` has a value it will return MaliputImplementation::kDragway.
// Otherwise it will return MaliputImplementation::kUnknown.
//
// @param filename Is the path to the YAML file.
// @param dragway_build_properties Contains the properties needed to build a dragway::RoadGeometry.
// @returns The MaliputImplementation choosen.
MaliputImplementation GetMaliputImplementation(const std::optional<std::string>& filename,
                                               const std::optional<DragwayBuildProperties>& dragway_build_properties) {
  if (filename.has_value() && dragway_build_properties.has_value()) {
    log()->error("Both implementations selected.");
    return MaliputImplementation::kUnknown;
  }
  return filename.has_value() ? (IsMultilaneDescription(filename.value()) ? MaliputImplementation::kMultilane
                                                                          : MaliputImplementation::kUnknown)
                              : (dragway_build_properties.has_value() ? MaliputImplementation::kDragway
                                                                      : MaliputImplementation::kUnknown);
}

}  // namespace

std::unique_ptr<const api::RoadGeometry> CreateRoadGeometryFrom(
    const std::optional<std::string>& filename, const std::optional<DragwayBuildProperties>& dragway_build_properties) {
  switch (GetMaliputImplementation(filename, dragway_build_properties)) {
    case MaliputImplementation::kDragway: {
      maliput::log()->debug("About to create dragway RoadGeometry.");
      return std::make_unique<dragway::RoadGeometry>(
          api::RoadGeometryId{"Dragway with " + std::to_string(dragway_build_properties.value().num_lanes) + " lanes."},
          dragway_build_properties.value().num_lanes, dragway_build_properties.value().length,
          dragway_build_properties.value().lane_width, dragway_build_properties.value().shoulder_width,
          dragway_build_properties.value().maximum_height, std::numeric_limits<double>::epsilon(),
          std::numeric_limits<double>::epsilon());
      break;
    }
    case MaliputImplementation::kMultilane: {
      maliput::log()->debug("About to load multilane RoadGeometry.");
      return maliput::multilane::LoadFile(maliput::multilane::BuilderFactory(), filename.value());
      break;
    }
    case MaliputImplementation::kUnknown: {
      maliput::log()->error("Unknown map.");
      return nullptr;
      break;
    }
    default:
      maliput::log()->error("GetMaliputImplementation() method returned an unexpected value.");
      return nullptr;
  }
}

}  // namespace integration
}  // namespace maliput
