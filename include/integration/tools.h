#pragma once

#include "maliput/api/road_geometry.h"
#include "maliput/api/road_network.h"

#include <memory>
#include <optional>
#include <string>

namespace maliput {
namespace integration {

/// Available maliput implementations.
enum class MaliputImplementation {
  kMalidrive,  //< malidrive implementation.
  kDragway,    //< dragway implementation.
  kMultilane,  //< multilane implementation.
};

/// Returns the std::string version of `maliput_impl`.
std::string MaliputImplementationToString(MaliputImplementation maliput_impl);

/// Returns the MaliputImplementation version of `maliput_impl`.
MaliputImplementation StringToMaliputImplementation(const std::string& maliput_impl);

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

/// Contains the attributes needed for building a multilane::Roadgeometry.
struct MultilaneBuildProperties {
  std::string yaml_file{""};
};

/// Contains the attributes needed for building a malidrive::Roadgeometry.
struct MalidriveBuildProperties {
  std::string xodr_file_path{""};
  double linear_tolerance{5e-2};
};

/// Builds an api::RoadGeometry based on Dragway implementation.
/// @param build_properties Holds the properties to build the RoadGeometry.
/// @return A maliput::api::RoadGeometry.
std::unique_ptr<const api::RoadGeometry> CreateDragwayRoadGeometry(const DragwayBuildProperties& build_properties);

/// Builds an api::RoadGeometry based on Multilane implementation.
/// @param build_properties Holds the properties to build the RoadGeometry.
/// @return A maliput::api::RoadGeometry.
///
/// @throw maliput::common::assertion_error When `build_properties.yaml_file` is empty.
std::unique_ptr<const api::RoadGeometry> CreateMultilaneRoadGeometry(const MultilaneBuildProperties& build_properties);

/// Builds an api::RoadNetwork based on Malidrive implementation.
/// @param build_properties Holds the properties to build the RoadNetwork.
/// @return A maliput::api::RoadNetwork.
///
/// @throw maliput::common::assertion_error When `build_properties.xodr_file_path` is empty.
std::unique_ptr<const api::RoadNetwork> CreateMalidriveRoadNetwork(const MalidriveBuildProperties& build_properties);

}  // namespace integration
}  // namespace maliput
