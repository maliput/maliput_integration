#pragma once

#include "maliput/api/road_geometry.h"

#include <memory>
#include <optional>
#include <string>

namespace maliput {
namespace integration {

/// Contains the attributes needed for building a dragway::Roadgeometry.
struct DragwayBuildProperties {
  /// Number of lanes.
  int num_lanes{2};
  /// Length of the lanes.
  double length{10};
  /// Width of the lanes.
  double lane_width{3.7};
  /// Width of the shoulders of the road.
  double shoulder_width{3.};
  /// Maximum height above the road surface.
  double maximum_height{5.2};
};

/// Builds an api::RoadGeometry based on the following rules:
///
/// - `filename` is set and `dragway_build_properties` is not, and `filename` points to a valid `multilane` YAML.
///    It creates a multilane::RoadGeometry.
/// - `filename` is not set and `dragway_build_properties` is, and `dragway_build_properties` has valid attributes.
///    It creates a dragway::RoadGeometry.
///
/// @param filename Is the YAML file containing a road geometry description to be built as a multilane::RoadGeometry.
/// @param dragway_build_properties Contains the properties needeed to build a dragway::RoadGeometry.
/// @return When attributes are valid and mutually exclusive, a valid maliput::api::RoadGeometry based on the
/// appropriate backend. Otherwise, nullptr.
std::unique_ptr<const api::RoadGeometry> CreateRoadGeometryFrom(
    const std::optional<std::string>& filename, const std::optional<DragwayBuildProperties>& dragway_build_properties);

}  // namespace integration
}  // namespace maliput
