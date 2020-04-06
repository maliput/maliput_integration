#pragma once

#include "maliput/api/road_geometry.h"

#include <memory>
#include <optional>
#include <string>

namespace maliput {
namespace integration {

/// Selects a RoadGeometry implementation and build the road geometry.
/// If `filename` is not empty it will try to build a multilane::RoadGeometry.
/// If `filename` is empty it will try to build a dragway::RoadGeometry using the remaining arguments.
///
/// @param filename Is the YAML file containing a road geometry description to be built as a multilane::RoadGeometry.
/// @param num_lanes Is the number of lanes for a dragway::RoadGeometry.
/// @param length Is the length of the lanes for a dragway::RoadGeometry.
/// @param lane_width Is width of the lanes for a dragway::RoadGeometry.
/// @param shoulder_width Is width of the shoulders for a dragway::RoadGeometry.
/// @param maximum_height  The maximum height above the road surface for a dragway::RoadGeometry.
/// @returns A maliput::api::RoadGeometry when it could be built.
///          A std::nullopt when no implementation is matched.
/// @throw maliput::common::assertion_error When `filename` is empty and `num_lanes` or `length` or `lane_width` or
/// `shoulder_width` or `maximum_height` is negative.
std::optional<std::unique_ptr<const api::RoadGeometry>> CreateRoadGeometryFrom(const std::string& filename,
                                                                               int num_lanes, double length,
                                                                               double lane_width, double shoulder_width,
                                                                               double maximum_height);

}  // namespace integration
}  // namespace maliput
