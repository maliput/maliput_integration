#pragma once

#include "maliput/api/road_geometry.h"

#include <memory>
#include <optional>
#include <string>

namespace maliput {
namespace integration {

/// Contains the attributes needed for building a dragway::Roadgeometry.
struct DragwayBuildProperties {
  int num_lanes{};
  double length{};
  double lane_width{};
  double shoulder_width{};
  double maximum_height{};
};

/// Selects a RoadGeometry implementation and build the road geometry.
/// If only `filename` has a value it will try to build a multilane::RoadGeometry.
/// If only `dragway_build_properties` has a value itwill try to build a dragway::RoadGeometry
/// Otherwise it will return nullptr.
///
/// @param filename Is the YAML file containing a road geometry description to be built as a multilane::RoadGeometry.
/// @param dragway_build_properties Contains the properties needeed to build a dragway::RoadGeometry.
/// @returns A maliput::api::RoadGeometry when it could be built.
///          A nullptr when no implementation is matched.
std::unique_ptr<const api::RoadGeometry> CreateRoadGeometryFrom(
    const std::optional<std::string>& filename, const std::optional<DragwayBuildProperties>& dragway_build_properties);

}  // namespace integration
}  // namespace maliput
