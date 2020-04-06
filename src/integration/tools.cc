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
// If `filename` is empty it returns MaliputImplementation:kDragway.
// If `filename` is not empty it will load the file and check which implementation it refers to.
//
// @param filename Is the path to the filename.
// @returns The MaliputImplementation choosen.
MaliputImplementation GetMaliputImplementation(const std::string& filename) {
  return filename.empty()
             ? MaliputImplementation::kDragway
             : (IsMultilaneDescription(filename) ? MaliputImplementation::kMultilane : MaliputImplementation::kUnknown);
}

}  // namespace

std::optional<std::unique_ptr<const api::RoadGeometry>> CreateRoadGeometryFrom(const std::string& filename,
                                                                               int num_lanes, double length,
                                                                               double lane_width, double shoulder_width,
                                                                               double maximum_height) {
  switch (GetMaliputImplementation(filename)) {
    case MaliputImplementation::kDragway: {
      maliput::log()->debug("Loaded a dragway road geometry.");
      return std::make_unique<dragway::RoadGeometry>(
          api::RoadGeometryId{"Dragway with " + std::to_string(num_lanes) + " lanes."}, num_lanes, length, lane_width,
          shoulder_width, maximum_height, std::numeric_limits<double>::epsilon(),
          std::numeric_limits<double>::epsilon());
      break;
    }
    case MaliputImplementation::kMultilane: {
      maliput::log()->debug("Loaded a multilane road geometry.");
      return maliput::multilane::LoadFile(maliput::multilane::BuilderFactory(), filename);
      break;
    }
    case MaliputImplementation::kUnknown: {
      maliput::log()->error("Unknown map.");
      return std::nullopt;
    }
    default:
      return std::nullopt;
  }
}

}  // namespace integration
}  // namespace maliput
