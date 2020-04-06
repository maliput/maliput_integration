#pragma once

#include "maliput/api/road_geometry.h"

#include <memory>
#include <optional>
#include <string>

namespace maliput {
namespace integration {

/// Available maliput implementations.
enum class MaliputImplementation {
  kDragway,    //< dragway implementation.
  kMultilane,  //< multilane implementation.
  kUnknown     //< Used when none of the implementations could be identified.
};

/// Parses a file whose path is `filename` as a YAML and looks for a node called
/// "maliput_multilane_builder". If it is found,
/// MaliputImplementation::kMultilane is returned. Otherwise,
/// MaliputImplementation::kUnknown is returned.
///
/// @param filename Is the path to the filename.
/// @returns The MaliputImplementation from the file.
/// @throw maliput::common::assertion_error When YAML file is not well-constructed.
MaliputImplementation GetMaliputImplementationFromFile(const std::string& filename);

/// Decides which type of road geometry implementation should be choosen.
/// If `filename` is empty it returns MaliputImplementation:kDragway.
/// If `filename` is not empty it will load the file and check which implementation it refers to.
///
/// @param filename Is the path to the filename.
/// @returns The MaliputImplementation choosen.
MaliputImplementation DecideMaliputImplementation(const std::string& filename);

/// Selects a RoadGeometry implementation and build the road geometry.
/// If `yaml_file` is not empty it will try to build a multilane::RoadGeometry.
/// If `yaml_file` is empty it will try to build a dragway::RoadGeometry using the remaining arguments.
///
/// @param yaml_file Is the YAML file containing a road geometry description to be built as a multilane::RoadGeometry.
/// @param num_lanes Is the number of lanes for a dragway::RoadGeometry.
/// @param length Is the length of the lanes for a dragway::RoadGeometry.
/// @param lane_width Is width of the lanes for a dragway::RoadGeometry.
/// @param shoulder_width Is width of the shoulders for a dragway::RoadGeometry.
/// @param maximum_height  The maximum height above the road surface for a dragway::RoadGeometry.
/// @returns A maliput::api::RoadGeometry when it could be built.
///          A std::nullopt when no implementation is matched.
/// @throw maliput::common::assertion_error When `yaml_file` is empty and `num_lanes` or `length` or `lane_width` or
/// `shoulder_width` or `maximum_height` is negative.
std::optional<std::unique_ptr<const api::RoadGeometry>> CreateRoadGeometryFrom(std::string yaml_file, int num_lanes,
                                                                               double length, double lane_width,
                                                                               double shoulder_width,
                                                                               double maximum_height);

}  // namespace integration
}  // namespace maliput
