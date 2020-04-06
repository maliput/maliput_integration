#include "integration/tools.h"

#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput_dragway/road_geometry.h"

#include "maliput_multilane/builder.h"
#include "maliput_multilane/loader.h"

#include "yaml-cpp/yaml.h"

namespace maliput {
namespace integration {

MaliputImplementation GetMaliputImplementationFromFile(const std::string& filename) {
  const YAML::Node yaml_file = YAML::LoadFile(filename);
  MALIPUT_DEMAND(yaml_file.IsMap());
  return yaml_file["maliput_multilane_builder"] ? MaliputImplementation::kMultilane : MaliputImplementation::kUnknown;
}

MaliputImplementation DecideMaliputImplementation(const std::string& filename) {
  return filename.empty() ? MaliputImplementation::kDragway : GetMaliputImplementationFromFile(filename);
}

std::optional<std::unique_ptr<const api::RoadGeometry>> CreateRoadGeometryFrom(std::string yaml_file, int num_lanes,
                                                                               double length, double lane_width,
                                                                               double shoulder_width,
                                                                               double maximum_height) {
  switch (DecideMaliputImplementation(yaml_file)) {
    case MaliputImplementation::kDragway: {
      maliput::log()->info("Loaded a dragway road geometry.");
      return std::make_unique<dragway::RoadGeometry>(
          api::RoadGeometryId{"Dragway with " + std::to_string(num_lanes) + " lanes."}, num_lanes, length, lane_width,
          shoulder_width, maximum_height, std::numeric_limits<double>::epsilon(),
          std::numeric_limits<double>::epsilon());
      break;
    }
    case MaliputImplementation::kMultilane: {
      maliput::log()->info("Loaded a multilane road geometry.");
      return maliput::multilane::LoadFile(maliput::multilane::BuilderFactory(), yaml_file);
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
