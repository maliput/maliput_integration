// BSD 3-Clause License
//
// Copyright (c) 2022, Woven Planet. All rights reserved.
// Copyright (c) 2020-2022, Toyota Research Institute. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/// @file maliput_dynamic_environment.cc
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
///   2. The application allows to select:
///      `-phase_duration`: the duration of each phase.
///      `-timeout`: the duration of the simulation.
///   3. The log level is selected with `-log_level`.

#include <map>
#include <memory>
#include <string>
#include <thread>

#include <gflags/gflags.h>
#include <maliput/base/rule_filter.h>
#include <maliput/base/rule_registry.h>
#include <maliput/common/logger.h>
#include <maliput/common/maliput_abort.h>

#include "integration/create_dynamic_environment_handler.h"
#include "integration/create_timer.h"
#include "integration/dynamic_environment_handler.h"
#include "integration/timer.h"
#include "integration/tools.h"
#include "maliput_gflags.h"

COMMON_PROPERTIES_FLAGS();
MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_OSM_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "malidrive",
              "Whether to use <dragway>, <multilane> or <malidrive>. Default is dragway.");
DEFINE_double(phase_duration, 2, "Duration of the phase in seconds.");
DEFINE_double(timeout, 20., "Timeout for calling off the simulation in seconds.");

namespace maliput {
namespace integration {
namespace {

using maliput::api::rules::DiscreteValueRule;
using maliput::api::rules::RangeValueRule;

// Obtains all the monostate DiscreteValueRules.
// @param rulebook RoadRulebook pointer.
std::map<DiscreteValueRule::Id, DiscreteValueRule> GetStaticDiscreteRules(
    const maliput::api::rules::RoadRulebook* rulebook) {
  const auto rules = rulebook->Rules();
  const auto discrete_value_rules = rules.discrete_value_rules;
  const auto filtered_rules = maliput::FilterRules(
      rulebook->Rules(), {[](const DiscreteValueRule& dvr) { return dvr.states().size() == 1; }}, {});
  return filtered_rules.discrete_value_rules;
}

// Obtains all the monostate RangeValueRules.
// @param rulebook RoadRulebook pointer.
std::map<RangeValueRule::Id, RangeValueRule> GetStaticRangeRules(const maliput::api::rules::RoadRulebook* rulebook) {
  std::vector<maliput::api::rules::RangeValueRule> static_rules;
  const auto rules = rulebook->Rules();
  const auto range_value_rules = rules.range_value_rules;
  const auto filtered_rules =
      maliput::FilterRules(rulebook->Rules(), {}, {[](const RangeValueRule& rvr) { return rvr.states().size() == 1; }});
  return filtered_rules.range_value_rules;
}

// Prints the states of all the static DiscreteValueRules: Rules with only one state.
// @param rn RoadNetwork pointer.
void PrintStaticDiscreteRulesStates(maliput::api::RoadNetwork* rn) {
  std::cout << "Static DiscreteValueRules" << std::endl;
  const auto static_discrete_value_rules = GetStaticDiscreteRules(rn->rulebook());
  // As there is only one state we could directly check the value of the DiscreteValueRule's values, however we use the
  // state provider instead in order to exercise good practices.
  const auto discrete_state_provider = rn->discrete_value_rule_state_provider();
  for (const auto& static_discrete_value_rule : static_discrete_value_rules) {
    std::cout << "\tDiscrete Value Rule: " << static_discrete_value_rule.first
              << " | State: " << discrete_state_provider->GetState(static_discrete_value_rule.first)->state.value
              << std::endl;
  }
}

// Prints the states of all the static RangeValueRules: Rules with only one state.
// @param rn RoadNetwork pointer.
void PrintStaticRangeRulesStates(maliput::api::RoadNetwork* rn) {
  std::cout << "Static RangeValueRules" << std::endl;
  const auto static_range_value_rules = GetStaticRangeRules(rn->rulebook());
  // As there is only one state we could directly check the value of the RangeValueRule's ranges, however we use the
  // state provider instead in order to exercise good practices.
  const auto range_state_provider = rn->range_value_rule_state_provider();
  for (const auto& static_range_value_rule : static_range_value_rules) {
    std::cout << "\tRange Value Rule: " << static_range_value_rule.first << " | State: ["
              << range_state_provider->GetState(static_range_value_rule.first)->state.min << ", "
              << range_state_provider->GetState(static_range_value_rule.first)->state.max << "]" << std::endl;
  }
}

// Prints the phase and the current states of Right-Of-Way rules and bulbs that are present in the phase rings.
// @param rn RoadNetwork pointer.
void PrintPhaseRingsCurrentStates(maliput::api::RoadNetwork* rn) {
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

int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  common::set_log_level(FLAGS_log_level);

  log()->info("Loading road network using ", FLAGS_maliput_backend, " backend implementation...");
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), GetAngularToleranceFlag(),
       FLAGS_build_policy, FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy,
       FLAGS_omit_nondrivable_lanes, FLAGS_integrator_accuracy_multiplier, FLAGS_rule_registry_file,
       FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file},
      {FLAGS_osm_file, FLAGS_linear_tolerance, FLAGS_max_linear_tolerance,
       maliput::math::Vector2::FromStr(FLAGS_origin), FLAGS_rule_registry_file, FLAGS_road_rule_book_file,
       FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file, FLAGS_intersection_book_file});
  log()->info("RoadNetwork loaded successfully.");

  const std::unique_ptr<const Timer> timer = CreateTimer(TimerType::kChronoTimer);
  const std::unique_ptr<DynamicEnvironmentHandler> deh = CreateDynamicEnvironmentHandler(
      DynamicEnvironmentHandlerType::kFixedPhaseIterationHandler, timer.get(), rn.get(), FLAGS_phase_duration);

  // Obtains static rules.
  PrintStaticDiscreteRulesStates(rn.get());
  PrintStaticRangeRulesStates(rn.get());

  // Dynamics rules can also be queried via `DiscreteValueRuleStateProvider` and `RangeValueRuleStateProvider`.
  // In particular for the intersections, maliput provides some convenient classes to obtain the current phase which
  // matches with current states in the Right-Of-Way Rule Type rules and bulb states that are present.
  while (timer->Elapsed() <= FLAGS_timeout) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::cout << "Time: " << timer->Elapsed() << std::endl;
    deh->Update();
    PrintPhaseRingsCurrentStates(rn.get());
  }

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
