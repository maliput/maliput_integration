#include "integration/tools.h"

#include <map>

#include "maliput/base/intersection_book.h"
#include "maliput/base/intersection_book_loader.h"
#include "maliput/base/manual_discrete_value_rule_state_provider.h"
#include "maliput/base/manual_phase_provider.h"
#include "maliput/base/manual_phase_ring_book.h"
#include "maliput/base/manual_range_value_rule_state_provider.h"
#include "maliput/base/manual_right_of_way_rule_state_provider.h"
#include "maliput/base/manual_rulebook.h"
#include "maliput/base/phase_ring_book_loader.h"
#include "maliput/base/road_rulebook_loader.h"
#include "maliput/base/traffic_light_book.h"
#include "maliput/base/traffic_light_book_loader.h"
#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput_dragway/road_geometry.h"

#include "maliput_malidrive/base/inertial_to_lane_mapping_config.h"
#include "maliput_malidrive/builder/road_network_builder.h"
#include "maliput_malidrive/loader/loader.h"

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

std::unique_ptr<const api::RoadNetwork> CreateDragwayRoadNetwork(const DragwayBuildProperties& build_properties) {
  maliput::log()->debug("Building dragway RoadNetwork.");
  auto rg = std::make_unique<dragway::RoadGeometry>(
      api::RoadGeometryId{"Dragway with " + std::to_string(build_properties.num_lanes) + " lanes."},
      build_properties.num_lanes, build_properties.length, build_properties.lane_width, build_properties.shoulder_width,
      build_properties.maximum_height, std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon(),
      maliput::math::Vector3(0, 0, 0));

  std::unique_ptr<ManualRulebook> rulebook = std::make_unique<ManualRulebook>();
  std::unique_ptr<TrafficLightBook> traffic_light_book = std::make_unique<TrafficLightBook>();
  std::unique_ptr<api::rules::RuleRegistry> rule_registry = std::make_unique<api::rules::RuleRegistry>();
  std::unique_ptr<ManualPhaseRingBook> phase_ring_book = std::make_unique<ManualPhaseRingBook>();
  std::unique_ptr<ManualPhaseProvider> phase_provider = std::make_unique<ManualPhaseProvider>();
  std::unique_ptr<IntersectionBook> intersection_book = std::make_unique<IntersectionBook>();

  std::unique_ptr<ManualRightOfWayRuleStateProvider> right_of_way_rule_state_provider =
      std::make_unique<ManualRightOfWayRuleStateProvider>();
  std::unique_ptr<ManualDiscreteValueRuleStateProvider> discrete_value_rule_state_provider =
      std::make_unique<ManualDiscreteValueRuleStateProvider>(rulebook.get());
  std::unique_ptr<ManualRangeValueRuleStateProvider> range_value_rule_state_provider =
      std::make_unique<ManualRangeValueRuleStateProvider>(rulebook.get());
  return std::make_unique<api::RoadNetwork>(std::move(rg), std::move(rulebook), std::move(traffic_light_book),
                                            std::move(intersection_book), std::move(phase_ring_book),
                                            std::move(right_of_way_rule_state_provider), std::move(phase_provider),
                                            std::move(rule_registry), std::move(discrete_value_rule_state_provider),
                                            std::move(range_value_rule_state_provider));
}

std::unique_ptr<const api::RoadNetwork> CreateMultilaneRoadNetwork(const MultilaneBuildProperties& build_properties) {
  maliput::log()->debug("Building multilane RoadNetwork.");
  if (build_properties.yaml_file.empty()) {
    MALIPUT_ABORT_MESSAGE("yaml_file cannot be empty.");
  }
  auto rg = maliput::multilane::LoadFile(maliput::multilane::BuilderFactory(), build_properties.yaml_file);
  auto rulebook = LoadRoadRulebookFromFile(rg.get(), build_properties.yaml_file);
  auto traffic_light_book = LoadTrafficLightBookFromFile(build_properties.yaml_file);
  auto phase_ring_book =
      LoadPhaseRingBookFromFile(rulebook.get(), traffic_light_book.get(), build_properties.yaml_file);
  std::unique_ptr<ManualPhaseProvider> phase_provider = std::make_unique<ManualPhaseProvider>();
  auto intersection_book =
      LoadIntersectionBookFromFile(build_properties.yaml_file, *rulebook, *phase_ring_book, phase_provider.get());
  std::unique_ptr<api::rules::RuleRegistry> rule_registry = std::make_unique<api::rules::RuleRegistry>();

  std::unique_ptr<ManualRightOfWayRuleStateProvider> right_of_way_rule_state_provider =
      std::make_unique<ManualRightOfWayRuleStateProvider>();
  std::unique_ptr<ManualDiscreteValueRuleStateProvider> discrete_value_rule_state_provider =
      std::make_unique<ManualDiscreteValueRuleStateProvider>(rulebook.get());
  std::unique_ptr<ManualRangeValueRuleStateProvider> range_value_rule_state_provider =
      std::make_unique<ManualRangeValueRuleStateProvider>(rulebook.get());
  return std::make_unique<api::RoadNetwork>(std::move(rg), std::move(rulebook), std::move(traffic_light_book),
                                            std::move(intersection_book), std::move(phase_ring_book),
                                            std::move(right_of_way_rule_state_provider), std::move(phase_provider),
                                            std::move(rule_registry), std::move(discrete_value_rule_state_provider),
                                            std::move(range_value_rule_state_provider));
}

std::unique_ptr<const api::RoadNetwork> CreateMalidriveRoadNetwork(const MalidriveBuildProperties& build_properties) {
  maliput::log()->debug("Building malidrive RoadNetwork.");
  const malidrive::builder::RoadGeometryConfiguration road_geometry_configuration{
      maliput::api::RoadGeometryId("malidrive_rg"),
      build_properties.xodr_file_path,
      build_properties.linear_tolerance,
      malidrive::constants::kAngularTolerance,
      malidrive::constants::kScaleLength,
      maliput::math::Vector3(0, 0, 0),
      malidrive::InertialToLaneMappingConfig(malidrive::constants::kExplorationRadius,
                                             malidrive::constants::kNumIterations),
      {malidrive::builder::BuildPolicy::FromStrToType(build_properties.build_policy),
       build_properties.number_of_threads == 0 ? std::nullopt
                                               : std::make_optional(build_properties.number_of_threads)}};
  if (!road_geometry_configuration.opendrive_file.has_value()) {
    MALIPUT_ABORT_MESSAGE("opendrive_file cannot be empty.");
  }
  const malidrive::builder::RoadNetworkConfiguration road_network_configuration{
      road_geometry_configuration,
      build_properties.road_rule_book_file.empty() ? std::nullopt
                                                   : std::make_optional(build_properties.road_rule_book_file),
      build_properties.traffic_light_book_file.empty() ? std::nullopt
                                                       : std::make_optional(build_properties.traffic_light_book_file),
      build_properties.phase_ring_book_file.empty() ? std::nullopt
                                                    : std::make_optional(build_properties.phase_ring_book_file),
      build_properties.intersection_book_file.empty() ? std::nullopt
                                                      : std::make_optional(build_properties.intersection_book_file)};
  return malidrive::loader::Load<malidrive::builder::RoadNetworkBuilder>(road_network_configuration);
}

std::unique_ptr<const api::RoadNetwork> LoadRoadNetwork(MaliputImplementation maliput_implementation,
                                                        const DragwayBuildProperties& dragway_build_properties,
                                                        const MultilaneBuildProperties& multilane_build_properties,
                                                        const MalidriveBuildProperties& malidrive_build_properties) {
  switch (maliput_implementation) {
    case MaliputImplementation::kDragway:
      return CreateDragwayRoadNetwork(dragway_build_properties);
    case MaliputImplementation::kMultilane:
      return CreateMultilaneRoadNetwork(multilane_build_properties);
    case MaliputImplementation::kMalidrive:
      return CreateMalidriveRoadNetwork(malidrive_build_properties);
    default:
      MALIPUT_ABORT_MESSAGE("Error loading RoadNetwork. Unknown implementation.");
  }
}

}  // namespace integration
}  // namespace maliput
