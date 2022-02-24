// Copyright 2022 Toyota Research Institute.
/// @file maliput_to_string.cc
///
/// Builds an api::RoadNetwork and lists the rules which states change on a time basis.
/// Possible backends are `dragway`, `multilane` and `malidrive`.
///
/// @note
///   1. Allows to load a road geometry from different road geometry implementations.
///       The `maliput_backend` flag will determine the backend to be used.
///      - "dragway": The following flags are supported to use in order to create dragway road geometry:
///           -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///      - "multilane": yaml file path must be provided:
///           -yaml_file.
///      - "malidrive": xodr file path must be provided and the tolerance is optional:
///           -xodr_file_path -linear_tolerance.
///   2. The application allows to select the duration of each phase.
///      `-phase_duration`
///   3. The level of the logger is selected with `-log_level`.

#include <memory>
#include <string>
#include <thread>

#include <gflags/gflags.h>
#include <maliput/common/logger.h>
#include <maliput/common/maliput_abort.h>

#include "integration/create_dynamic_environment_handler.h"
#include "integration/create_timer.h"
#include "integration/dynamic_environment_handler.h"
#include "integration/timer.h"
#include "integration/tools.h"
#include "maliput_gflags.h"

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "malidrive",
              "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");
DEFINE_double(phase_duration, 2, "Duration of the phase in seconds.");

namespace maliput {
namespace integration {
namespace {

// Obtains all the monostate DiscreteValueRules.
// @param rulebook RoadRulebook pointer.
std::vector<maliput::api::rules::DiscreteValueRule> GetStaticDiscreteRules(
    const maliput::api::rules::RoadRulebook* rulebook) {
  std::vector<maliput::api::rules::DiscreteValueRule> static_rules;
  const auto rules = rulebook->Rules();
  const auto discrete_value_rules = rules.discrete_value_rules;
  std::for_each(discrete_value_rules.begin(), discrete_value_rules.end(),
                [&static_rules](const std::pair<maliput::api::rules::DiscreteValueRule::Id,
                                                maliput::api::rules::DiscreteValueRule>& discrete_value_rule) {
                  if (discrete_value_rule.second.values().size() == 1) {
                    static_rules.push_back(discrete_value_rule.second);
                  }
                });
  return static_rules;
}

// Obtains all the monostate RangeValueRules.
// @param rulebook RoadRulebook pointer.
std::vector<maliput::api::rules::RangeValueRule> GetStaticRangeRules(
    const maliput::api::rules::RoadRulebook* rulebook) {
  std::vector<maliput::api::rules::RangeValueRule> static_rules;
  const auto rules = rulebook->Rules();
  const auto range_value_rules = rules.range_value_rules;
  std::for_each(range_value_rules.begin(), range_value_rules.end(),
                [&static_rules](const std::pair<maliput::api::rules::RangeValueRule::Id,
                                                maliput::api::rules::RangeValueRule>& range_value_rule) {
                  if (range_value_rule.second.ranges().size() == 1) {
                    static_rules.push_back(range_value_rule.second);
                  }
                });
  return static_rules;
}

// Generates an OBJ file from a YAML file path or from
// configurable values given as CLI arguments.
int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  log()->info("Loading road network using {} backend implementation...", FLAGS_maliput_backend);
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), FLAGS_build_policy,
       FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy, FLAGS_omit_nondrivable_lanes,
       FLAGS_rule_registry_file, FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file});
  log()->info("RoadNetwork loaded successfully.");

  const std::unique_ptr<Timer> timer = CreateTimer(TimerType::kChronoTimer);
  std::unique_ptr<DynamicEnvironmentHandler> deh = CreateDynamicEnvironmentHandler(
      DynamicEnvironmentHandlerType::kFixedPhaseIterationHandler, timer.get(), rn.get(), FLAGS_phase_duration);

  // Obtains static rules.
  std::cout << "Static DiscreteValueRules" << std::endl;
  const auto static_discrete_value_rules = GetStaticDiscreteRules(rn->rulebook());
  for (const auto& static_discrete_value_rule : static_discrete_value_rules) {
    std::cout << "\tDiscrete Value Rule: " << static_discrete_value_rule.id()
              << " | State: " << static_discrete_value_rule.values()[0].value << std::endl;
  }
  std::cout << "Static RangeValueRules" << std::endl;
  const auto static_range_value_rules = GetStaticRangeRules(rn->rulebook());
  for (const auto& static_range_value_rule : static_range_value_rules) {
    std::cout << "\tRange Value Rule: " << static_range_value_rule.id() << " | State: ["
              << static_range_value_rule.ranges()[0].min << ", " << static_range_value_rule.ranges()[0].max << "]"
              << std::endl;
  }

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::cout << "Time: " << timer->Elapsed() << std::endl;
    deh->Update();
    // Obtains Phases via PhaseRingBook and their respectives Right-Of-Way DiscreteValueRules and BulbStates.
    for (const auto& phase_ring_id : rn->phase_ring_book()->GetPhaseRings()) {
      const auto current_phase_id{rn->phase_provider()->GetPhase(phase_ring_id)->state};
      std::cout << "PhaseRingId: " << phase_ring_id << " | Current Phase: " << current_phase_id << std::endl;
      const auto current_phase = rn->phase_ring_book()->GetPhaseRing(phase_ring_id)->GetPhase(current_phase_id);
      for (const auto& discrete_value_rule_state : current_phase->discrete_value_rule_states()) {
        std::cout << "\tDiscrete Value Rule: " << discrete_value_rule_state.first.string()
                  << " | State: " << discrete_value_rule_state.second.value << std::endl;
      }
      for (const auto& bulb_state : current_phase->bulb_states().value()) {
        std::cout << "\tBulbUniqueId: " << bulb_state.first.string()
                  << " | State: " << (bulb_state.second == maliput::api::rules::BulbState::kOn ? "On" : "Off")
                  << std::endl;
      }
    }
    // Obtaining the phase and other sensitive information via Intersection Book is recommended however you must have
    // define the intersections in the intersection book yaml file first.
    // @code{cpp}
    //   for (const auto& intersection : rn->intersection_book()->GetIntersections()) {
    //     const auto current_phase = intersection->Phase();
    //     const auto bulb_states = intersection->bulb_states();
    //     ...
    //   }
    // @endcode
  }

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
