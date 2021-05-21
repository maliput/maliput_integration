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

/// Contains the attributes needed for building a dragway::RoadGeometry.
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

/// Contains the attributes needed for building a multilane::RoadGeometry.
struct MultilaneBuildProperties {
  std::string yaml_file{""};
};

/// Contains the attributes needed for building a malidrive::RoadNetwork.
struct MalidriveBuildProperties {
  std::string xodr_file_path{""};
  double linear_tolerance{5e-2};
  std::string build_policy{"sequential"};
  int number_of_threads{0};
  std::string simplification_policy{"none"};
  std::string tolerance_selection_policy{"manual"};
  std::string standard_strictness_policy{"permissive"};
  std::string road_rule_book_file{""};
  std::string traffic_light_book_file{""};
  std::string phase_ring_book_file{""};
  std::string intersection_book_file{""};
};

/// Builds an api::RoadNetwork based on Dragway implementation.
/// @param build_properties Holds the properties to build the RoadNetwork.
/// @return A maliput::api::RoadNetwork.
std::unique_ptr<const api::RoadNetwork> CreateDragwayRoadNetwork(const DragwayBuildProperties& build_properties);

/// Builds an api::RoadNetwork based on Multilane implementation.
/// @param build_properties Holds the properties to build the RoadNetwork.
/// @return A maliput::api::RoadNetwork.
///
/// @throw maliput::common::assertion_error When `build_properties.yaml_file` is empty.
std::unique_ptr<const api::RoadNetwork> CreateMultilaneRoadNetwork(const MultilaneBuildProperties& build_properties);

/// Builds an api::RoadNetwork based on Malidrive implementation.
/// @param build_properties Holds the properties to build the RoadNetwork.
/// @return A maliput::api::RoadNetwork.
///
/// @throw maliput::common::assertion_error When `build_properties.xodr_file_path` is empty.
std::unique_ptr<const api::RoadNetwork> CreateMalidriveRoadNetwork(const MalidriveBuildProperties& build_properties);

/// Builds an api::RoadNetwork using the implementation that `maliput_implementation` describes.
/// @param maliput_implementation One of MaliputImplementation. (kDragway, kMultilane, kMalidrive).
/// @param dragway_build_properties Holds the properties to build a dragway RoadNetwork.
/// @param multilane_build_properties Holds the properties to build a multilane RoadNetwork.
/// @param malidrive_build_properties Holds the properties to build a malidrive RoadNetwork.
/// @return A maliput::api::RoadNetwork.
///
/// @throw maliput::common::assertion_error When `maliput_implementation` is unknown.
std::unique_ptr<const api::RoadNetwork> LoadRoadNetwork(MaliputImplementation maliput_implementation,
                                                        const DragwayBuildProperties& dragway_build_properties,
                                                        const MultilaneBuildProperties& multilane_build_properties,
                                                        const MalidriveBuildProperties& malidrive_build_properties);

}  // namespace integration
}  // namespace maliput
