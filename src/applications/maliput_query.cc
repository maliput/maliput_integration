// Copyright 2020 Toyota Research Institute.
/// @file maliput_query.cc
/// Convenient application to run maliput's queries against a dragway, multilane or malidrive's
/// RoadGeometry backend.
///
/// @note
/// 1. Allows to load a road geometry from different road geometry implementations.
///     The `maliput_backend` flag will determine the backend to be used.
///    - "dragway": The following flags are supported to use in order to create dragway road geometry:
///       -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
///    - "multilane": yaml file path must be provided:
///       -yaml_file.
///    - "malidrive": xodr file path must be provided, tolerance and rule file paths are optional:
///         -xodr_file_path -linear_tolerance -road_rule_book_file -traffic_light_book_file -phase_ring_book_file
///         -intersection_book_file
/// 2. The level of the logger could be setted by: -log_level.

#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <gflags/gflags.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

#include "maliput/common/logger.h"
#include "maliput/common/maliput_abort.h"

#include "maliput_malidrive/builder/road_network_configuration.h"
#include "maliput_malidrive/constants.h"
#include "maliput_malidrive/utility/file_tools.h"

MULTILANE_PROPERTIES_FLAGS();
DRAGWAY_PROPERTIES_FLAGS();
MALIDRIVE_PROPERTIES_FLAGS();
MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG();

DEFINE_string(maliput_backend, "malidrive",
              "Whether to use <dragway>, <multilane> or <malidrive>. Default is malidrive.");

namespace maliput {
namespace integration {
namespace {

struct Command {
  std::string name{"default"};
  std::string usage{"default"};
  // Descriptions are represented as a sequence of lines to
  // ease formatting (e.g. indentation, line wraps) in the
  // interest of readability.
  std::vector<std::string> description{{"default"}};
  int num_arguments{0};
};

// @returns A map of command name to usage message.
const std::map<const std::string, const Command> CommandsUsage() {
  return {
      {"FindRoadPositions",
       {"FindRoadPositions",
        "<xodr_file> FindRoadPositions x y z r",
        {"Obtains, for all Lanes whose segment regions include points",
         "that are within a radius r of an (x, y, z) InertialPosition i.e. in",
         "the world frame, the RoadPosition of the point in the Lane manifold",
         "which is closest to that InertialPosition."},
        6}},
      {"ToRoadPosition",
       {"ToRoadPosition",
        "<xodr_file> ToRoadPosition x y z",
        {"Obtains the RoadPosition of the point in the RoadGeometry manifold",
         "which is, in the world frame, closest to an (x, y, z) InertialPosition."},
        5}},
      {"ToLanePosition",
       {"ToLanePosition",
        "<xodr_file> ToLanePosition lane_id x y z",
        {"Obtains the LanePosition in a Lane, identified by lane_id, that is",
         "closest, in the world frame, to an (x, y, z) InertialPosition."},
        6}},
      {"GetOrientation",
       {"GetOrientation",
        "<xodr_file> GetOrientation lane_id s r h",
        {"Obtains the orientation in a Lane, identified by lane_id, that is",
         "closest, in the world frame, to an (s, r, h) LanePosition."},
        6}},
      {"LaneToInertialPosition",
       {"LaneToInertialPosition",
        "<xodr_file> LaneToInertialPosition lane_id s r h",
        {"Obtains the InertialPosition for an (s, r, h) LanePosition in a Lane,", "identified by lane_id."},
        6}},
      {"GetMaxSpeedLimit",
       {"GetMaxSpeedLimit",
        "<xodr_file> GetMaxSpeedLimit lane_id",
        {"Obtains the maximum SpeedLimitRule for a Lane identified by lane_id.",
         "Rules are defined on the RoadRuleBook as loaded from a "
         "--road_rule_book_file."},
        3}},
      {"GetDirectionUsage",
       {"GetDirectionUsage",
        "<xodr_file> GetDirectionUsage lane_id",
        {"Obtains all DirectionUsageRules for a Lane identified by lane_id.",
         "Rules are defined on the RoadRuleBook as loaded from a "
         "--road_rule_book_file."},
        3}},
      {"GetRightOfWay",
       {"GetRightOfWay",
        "<xodr_file> GetRightOfWay lane_id start_s end_s",
        {"Obtains all RightOfWayRules for a region [start_s, end_s] of a Lane,",
         "identified by lane_id. Rules are defined on the RoadRuleBook as loaded", "from a --road_rule_book_file."},
        5}},
      {"GetPhaseRightOfWay",
       {"GetPhaseRightOfWay",
        "<xodr_file> GetPhaseRightOfWay phase_ring_id phase_id",
        {"Obtains the state of RightOfWayRules for a Phase identified by"
         " phase_id",
         "in a PhaseRing identified by phase_ring_id.",
         "Rules are defined on the RoadRuleBook as loaded from "
         "a --road_rule_book_file.",
         "Phases are defined on the PhaseRingBook as loaded from "
         "a --phase_ring_book_file."},
        4}},
      {"GetDiscreteValueRules",
       {"GetDiscreteValueRules",
        "<xodr_file> GetDiscreteValueRules lane_id start_s end_s",
        {"Obtains all DiscreteValueRule for a region [start_s, end_s] of a Lane,",
         "identified by lane_id. Rules are defined on the RoadRuleBook as loaded",
         "from a --road_rule_book_file or the xodr itself."},
        5}},
      {"GetRangeValueRules",
       {"GetRangeValueRules",
        "<xodr_file> GetRangeValueRules lane_id start_s end_s",
        {"Obtains all RangeValueRules for a region [start_s, end_s] of a Lane,",
         "identified by lane_id. Rules are defined on the RoadRuleBook as loaded",
         "from a --road_rule_book_file or the xodr itself."},
        5}},
      {"GetLaneBounds",
       {"GetLaneBounds",
        "<xodr_file> GetLaneBounds lane_id s",
        {"Obtains the segment and lane bounds of lane_id at s position. Return strings would be: ",
         "[segment_bounds.min, lane_bounds.min, lane_bounds.max, segment_bounds.max]."},
        4}},
  };
}

// Returns a vector of all possible direction usage values. Item order
// matches maliput::api::rules::DirectionUsageRule::Type enumeration.
const std::vector<std::string> DirectionUsageRuleNames() {
  return {"WithS", "AgainstS", "Bidirectional", "BidirectionalTurnOnly", "NoUse", "Parking"};
};

// Serializes `road_position` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::RoadPosition& road_position) {
  return out << "(lane: " << road_position.lane->id().string() << ", lane_pos: " << road_position.pos << ")";
}

// Serializes `road_position_result` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::RoadPositionResult& road_position_result) {
  return out << "(road_pos:" << road_position_result.road_position
             << ", nearest_pos: " << road_position_result.nearest_position
             << ", distance: " << road_position_result.distance << ")";
}

// Serializes `state_type` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::rules::RightOfWayRule::State::Type& state_type) {
  switch (state_type) {
    case maliput::api::rules::RightOfWayRule::State::Type::kGo:
      out << "go";
      break;
    case maliput::api::rules::RightOfWayRule::State::Type::kStop:
      out << "stop";
      break;
    case maliput::api::rules::RightOfWayRule::State::Type::kStopThenGo:
      out << "stop then go";
      break;
    default:
      out << "unknown";
      break;
  }
  return out;
}

// Serializes `state` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::rules::RightOfWayRule::State& state) {
  out << "State(id: " << state.id().string() << ", type: '" << state.type() << "'"
      << ", yield group: [";
  for (const auto& right_of_way_rule_id : state.yield_to()) {
    out << right_of_way_rule_id.string() << ", ";
  }
  out << "])";
  return out;
}

// Serializes `s_range` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::SRange& s_range) {
  return out << "[" << s_range.s0() << ", " << s_range.s1() << "]";
}

// Serializes `lane_s_range` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::LaneSRange& lane_s_range) {
  return out << "Range(lane_id: " << lane_s_range.lane_id().string() << ", s_range:" << lane_s_range.s_range() << ")";
}

// Serializes `lane_s_route` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::LaneSRoute& lane_s_route) {
  out << "Route(ranges: [";
  for (const auto& range : lane_s_route.ranges()) {
    out << range << ", ";
  }
  return out << "])";
}

// Serializes `zone_type` into `out`.
std::ostream& operator<<(std::ostream& out, const maliput::api::rules::RightOfWayRule::ZoneType& zone_type) {
  switch (zone_type) {
    case maliput::api::rules::RightOfWayRule::ZoneType::kStopExcluded:
      out << "stop excluded";
      break;
    case maliput::api::rules::RightOfWayRule::ZoneType::kStopAllowed:
      out << "stop allowed";
      break;
    default:
      out << "unknown";
      break;
  }
  return out;
}

// Returns a string with the usage message.
std::string GetUsageMessage() {
  std::stringstream ss;
  ss << "CLI for easy Malidrive road networks querying" << std::endl << std::endl;
  ss << "  Supported commands:" << std::endl;
  const std::map<const std::string, const Command> command_usage = CommandsUsage();
  for (auto it = command_usage.begin(); it != command_usage.end(); ++it) {
    ss << "    " << it->second.usage << std::endl << std::endl;
    for (const std::string& line : it->second.description) {
      ss << "        " << line << std::endl;
    }
    ss << std::endl;
  }
  return ss.str();
}

/// Query and logs results to RoadGeometry or RoadRulebook minimizing the
/// overhead of getting the right calls / asserting conditions.
class RoadNetworkQuery {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(RoadNetworkQuery)

  /// Constructs a RoadNetworkQuery.
  ///
  /// @param out A pointer to an output stream where results will be logged.
  ///            It must not be nullptr.
  /// @param rn A pointer to a RoadNetwork. It must not be nullptr.
  /// @throws std::runtime_error When `out` or `rn` are nullptr.
  RoadNetworkQuery(std::ostream* out, maliput::api::RoadNetwork* rn) : out_(out), rn_(rn) {
    MALIPUT_THROW_UNLESS(out_ != nullptr);
    MALIPUT_THROW_UNLESS(rn_ != nullptr);
  }

  /// Redirects `inertial_position` and `radius` to RoadGeometry::FindRoadPosition().
  void FindRoadPositions(const maliput::api::InertialPosition& inertial_position, double radius) {
    const std::vector<maliput::api::RoadPositionResult> results =
        rn_->road_geometry()->FindRoadPositions(inertial_position, radius);

    (*out_) << "FindRoadPositions(inertial_position:" << inertial_position << ", radius: " << radius << ")"
            << std::endl;
    for (const maliput::api::RoadPositionResult& result : results) {
      (*out_) << "              : Result: " << result << std::endl;
    }
  }

  /// Redirects `lane_position` to `lane_id`'s Lane::ToInertialPosition().
  void ToInertialPosition(const maliput::api::LaneId& lane_id, const maliput::api::LanePosition& lane_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);

    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const maliput::api::InertialPosition inertial_position = lane->ToInertialPosition(lane_position);

    (*out_) << "(" << lane_id.string() << ")->ToInertialPosition(lane_position: " << lane_position << ")" << std::endl;
    (*out_) << "              : Result: inertial_position:" << inertial_position << std::endl;

    const maliput::api::RoadPositionResult result =
        rn_->road_geometry()->ToRoadPosition(inertial_position, std::nullopt);

    (*out_) << "              : Result round_trip inertial_position" << result.nearest_position
            << ", with distance: " << result.distance << std::endl;
    (*out_) << "              : RoadPosition: " << result.road_position << std::endl;
  }

  /// Redirects `inertial_position` to `lane_id`'s Lane::ToLanePosition().
  void ToLanePosition(const maliput::api::LaneId& lane_id, const maliput::api::InertialPosition& inertial_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const maliput::api::LanePositionResult lane_position_result = lane->ToLanePosition(inertial_position);

    (*out_) << "(" << lane_id.string() << ")->ToLanePosition(inertial_position: " << inertial_position << ")"
            << std::endl;
    (*out_) << "              : Result: lane_pos:" << lane_position_result.lane_position
            << ", nearest_pos: " << lane_position_result.nearest_position
            << ", with distance: " << lane_position_result.distance << std::endl;
  }

  /// Redirects `lane_position` to `lane_id`'s Lane::GetOrientation().
  void GetOrientation(const maliput::api::LaneId& lane_id, const maliput::api::LanePosition& lane_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);

    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const maliput::api::Rotation rotation = lane->GetOrientation(lane_position);

    (*out_) << "(" << lane_id.string() << ")->GetOrientation(lane_position: " << lane_position << ")" << std::endl;
    (*out_) << "              : Result: orientation:" << rotation << std::endl;
  }

  /// Redirects `inertial_position` to RoadGeometry::ToRoadPosition().
  void ToRoadPosition(const maliput::api::InertialPosition& inertial_position) {
    const maliput::api::RoadPositionResult result = rn_->road_geometry()->ToRoadPosition(inertial_position);

    (*out_) << "ToRoadPosition(inertial_position: " << inertial_position << ")" << std::endl;
    (*out_) << "              : Result: nearest_pos:" << result.nearest_position
            << " with distance: " << result.distance << std::endl;
    (*out_) << "                RoadPosition: " << result.road_position << std::endl;
  }

  /// Looks for all the maximum speed limits allowed at `lane_id`.
  void GetMaxSpeedLimit(const maliput::api::LaneId& lane_id) {
    const maliput::api::rules::RoadRulebook::QueryResults query_result = FindRulesFor(lane_id);

    const int n_speed_limits = static_cast<int>(query_result.speed_limit.size());
    if (n_speed_limits > 0) {
      double max_speed = query_result.speed_limit.begin()->second.max();
      maliput::api::rules::SpeedLimitRule::Id max_speed_id = query_result.speed_limit.begin()->first;
      for (auto const& speed_val : query_result.speed_limit) {
        const double max_speed_cur = speed_val.second.max();
        if (max_speed_cur < max_speed) {
          max_speed = max_speed_cur;
          max_speed_id = speed_val.first;
        }
      }
      (*out_) << "Speed limit (" << max_speed_id.string() << "): " << max_speed << " m/s" << std::endl;
    } else {
      (*out_) << "There is no speed limit found for this lane" << std::endl;
    }
  }

  /// Looks for all the direction usages at `lane_id`.
  void GetDirectionUsage(const maliput::api::LaneId& lane_id) {
    const maliput::api::rules::RoadRulebook::QueryResults query_result = FindRulesFor(lane_id);

    const int n_rules = static_cast<int>(query_result.direction_usage.size());
    const std::vector<std::string> direction_usage_names = DirectionUsageRuleNames();

    if (n_rules > 0) {
      for (const auto& direction_rule : query_result.direction_usage) {
        const auto& states = direction_rule.second.states();
        for (const auto& state : states) {
          const int state_type = int(state.second.type());
          if (state_type < 0 || state_type >= int(direction_usage_names.size())) {
            (*out_) << "              : Result: Invalid direction usage rule. " << std::endl;
            return;
          }

          (*out_) << "              : Result: Rule (" << direction_rule.second.id().string()
                  << "): " << direction_usage_names[state_type] << std::endl;
        }
      }
    } else {
      (*out_) << "              : Result: There is no direction usage rules "
              << "found for this lane" << std::endl;
    }
  }

  /// Gets all right-of-way rules for the given `lane_s_range`.
  void GetRightOfWay(const maliput::api::LaneSRange& lane_s_range) {
    const maliput::api::rules::RoadRulebook::QueryResults results = rn_->rulebook()->FindRules({lane_s_range}, 0.);
    maliput::api::rules::RightOfWayRuleStateProvider* right_of_way_rule_state_provider =
        rn_->right_of_way_rule_state_provider();
    (*out_) << "Right of way for " << lane_s_range << ":" << std::endl;
    for (const auto& rule : results.right_of_way) {
      (*out_) << "    Rule(id: " << rule.second.id().string() << ", zone: " << rule.second.zone() << ", zone-type: '"
              << rule.second.zone_type() << "'";
      if (!rule.second.is_static()) {
        (*out_) << ", states: [";
        for (const auto& entry : rule.second.states()) {
          (*out_) << entry.second << ", ";
        }
        (*out_) << "]";
        auto rule_state_result = right_of_way_rule_state_provider->GetState(rule.second.id());
        if (rule_state_result.has_value()) {
          auto it = rule.second.states().find(rule_state_result->state);
          MALIPUT_DEMAND(it != rule.second.states().end());
          (*out_) << ", current_state: " << it->second;
        }
      } else {
        (*out_) << ", current_state: " << rule.second.static_state();
      }
      (*out_) << ", static: " << (rule.second.is_static() ? "yes" : "no") << ")" << std::endl << std::endl;
    }
  }

  /// Gets all discrete-value-rules rules for the given `lane_s_range`.
  void GetDiscreteValueRule(const maliput::api::LaneSRange& lane_s_range) {
    const maliput::api::rules::RoadRulebook::QueryResults results = rn_->rulebook()->FindRules({lane_s_range}, 0.);
    maliput::api::rules::DiscreteValueRuleStateProvider* state_provider = rn_->discrete_value_rule_state_provider();
    (*out_) << "DiscreteValueRules for " << lane_s_range << ":" << std::endl;
    for (const auto& rule : results.discrete_value_rules) {
      const std::optional<maliput::api::rules::DiscreteValueRuleStateProvider::StateResult> rule_state =
          state_provider->GetState(rule.second.id());

      (*out_) << "    Rule(id: " << rule.second.id().string() << ", zone: " << rule.second.zone();
      if (rule_state.has_value()) {
        (*out_) << ", state: (severity: " << rule_state->state.severity << ", related rules: [";
        for (const auto& related_rule_group : rule_state->state.related_rules) {
          (*out_) << related_rule_group.first << ": {";
          for (const auto& rule_id : related_rule_group.second) {
            (*out_) << rule_id.string() << ", ";
          }
          (*out_) << "}";
        }
        (*out_) << "], value: " << rule_state->state.value;
      }

      (*out_) << ")" << std::endl << std::endl;
    }
  }

  /// Gets all range-value-rules rules for the given `lane_s_range`.
  void GetRangeValueRule(const maliput::api::LaneSRange& lane_s_range) {
    const maliput::api::rules::RoadRulebook::QueryResults results = rn_->rulebook()->FindRules({lane_s_range}, 0.);
    maliput::api::rules::RangeValueRuleStateProvider* state_provider = rn_->range_value_rule_state_provider();
    (*out_) << "RangeValueRules for " << lane_s_range << ":" << std::endl;
    for (const auto& rule : results.range_value_rules) {
      const std::optional<maliput::api::rules::RangeValueRuleStateProvider::StateResult> rule_state =
          state_provider->GetState(rule.second.id());

      (*out_) << "    Rule(id: " << rule.second.id().string() << ", zone: " << rule.second.zone();
      if (rule_state.has_value()) {
        (*out_) << ", state: (severity: " << rule_state->state.severity << ", related rules: [";
        for (const auto& related_rule_group : rule_state->state.related_rules) {
          (*out_) << related_rule_group.first << ": {";
          for (const auto& rule_id : related_rule_group.second) {
            (*out_) << rule_id.string() << ", ";
          }
          (*out_) << "}";
        }
        (*out_) << "], description: " << rule_state->state.description << ", min: " << rule_state->state.min
                << ", max: " << rule_state->state.max;
      }

      (*out_) << ")" << std::endl << std::endl;
    }
  }

  /// Gets all right-of-way rules' states for a given phase in a given phase
  /// ring.
  void GetPhaseRightOfWay(const maliput::api::rules::PhaseRing::Id& phase_ring_id,
                          const maliput::api::rules::Phase::Id& phase_id) {
    const maliput::api::rules::PhaseRingBook* phase_ring_book = rn_->phase_ring_book();
    if (phase_ring_book == nullptr) {
      (*out_) << "Road network has no phase ring book" << std::endl;
      return;
    }

    const maliput::api::rules::RoadRulebook* road_rule_book = rn_->rulebook();
    if (road_rule_book == nullptr) {
      (*out_) << "Road network has no road rule book" << std::endl;
      return;
    }

    std::optional<maliput::api::rules::PhaseRing> phase_ring = phase_ring_book->GetPhaseRing(phase_ring_id);
    if (!phase_ring.has_value()) {
      (*out_) << "'" << phase_ring_id.string() << "' is not a known phase ring" << std::endl;
      return;
    }

    auto it = phase_ring->phases().find(phase_id);
    if (it == phase_ring->phases().end()) {
      (*out_) << "'" << phase_id.string() << "' is not a phase in phase ring '" << phase_ring_id.string() << "'"
              << std::endl;
      return;
    }

    const maliput::api::rules::Phase& phase = it->second;
    (*out_) << "Right of way for " << phase_id.string() << ":" << std::endl;
    for (const auto& rule_id_to_rule_state_id : phase.rule_states()) {
      const maliput::api::rules::RightOfWayRule rule = road_rule_book->GetRule(rule_id_to_rule_state_id.first);
      const maliput::api::rules::RightOfWayRule::State& rule_state = rule.states().at(rule_id_to_rule_state_id.second);
      (*out_) << "    Rule(id: " << rule.id().string() << ", zone: " << rule.zone() << ", zone-type: '"
              << rule.zone_type() << "'"
              << ", current_state: " << rule_state << ", static: " << (rule.is_static() ? "yes" : "no") << ")"
              << std::endl
              << std::endl;
    }
  }

  /// Gets a lane boundaries for `lane_id` at `s`.
  void GetLaneBounds(const maliput::api::LaneId& lane_id, double s) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    if (lane == nullptr) {
      std::cerr << " Could not find lane. " << std::endl;
      return;
    }
    const maliput::api::RBounds segment_bounds = lane->segment_bounds(s);
    const maliput::api::RBounds lane_bounds = lane->lane_bounds(s);
    (*out_) << "Lateral boundaries for  " << lane_id.string() << ":" << std::endl
            << "    [" << segment_bounds.min() << "; " << lane_bounds.min() << "; " << lane_bounds.max() << "; "
            << segment_bounds.max() << "]" << std::endl;
  }

 private:
  // Finds QueryResults of Rules for `lane_id`.
  maliput::api::rules::RoadRulebook::QueryResults FindRulesFor(const maliput::api::LaneId& lane_id) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    if (lane == nullptr) {
      std::cerr << " Could not find lane. " << std::endl;
      return maliput::api::rules::RoadRulebook::QueryResults();
    }

    const maliput::api::SRange s_range(0., lane->length());
    const maliput::api::LaneSRange lane_s_range(lane->id(), s_range);
    const std::vector<maliput::api::LaneSRange> lane_s_ranges(1, lane_s_range);

    return rn_->rulebook()->FindRules(lane_s_ranges, 0.);
  }

  std::ostream* out_{};
  maliput::api::RoadNetwork* rn_{};
};

/// @return A LaneId whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::LaneId LaneIdFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);
  return maliput::api::LaneId(std::string(*argv));
}

/// @return A PhaseRing::Id whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::rules::PhaseRing::Id PhaseRingIdFromCLI(char** argv) {
  return maliput::api::rules::PhaseRing::Id(std::string(*argv));
}

/// @return A Phase::Id whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::rules::Phase::Id PhaseIdFromCLI(char** argv) {
  return maliput::api::rules::Phase::Id(std::string(*argv));
}

/// @return An SRange whose string representation is a sequence 's0 s1'
///         pointed by `argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::SRange SRangeFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);

  const double s0 = std::strtod(argv[0], nullptr);
  const double s1 = std::strtod(argv[1], nullptr);
  return maliput::api::SRange(s0, s1);
}

/// @return A LaneSRange whose string representation is a sequence
///         'lane_id s0 s1' pointed by `argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::LaneSRange LaneSRangeFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);

  return maliput::api::LaneSRange(LaneIdFromCLI(argv), SRangeFromCLI(&(argv[1])));
}

/// @return A LanePosition whose string representation is a sequence 's r h'
///         pointed by `argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::LanePosition LanePositionFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);

  const double s = std::strtod(argv[0], nullptr);
  const double r = std::strtod(argv[1], nullptr);
  const double h = std::strtod(argv[2], nullptr);
  return maliput::api::LanePosition(s, r, h);
}

/// @return A InertialPosition whose string representation is a sequence 'x y z'
///         pointed by `argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::InertialPosition InertialPositionFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);

  const double x = std::strtod(argv[0], nullptr);
  const double y = std::strtod(argv[1], nullptr);
  const double z = std::strtod(argv[2], nullptr);
  return maliput::api::InertialPosition(x, y, z);
}

/// @return A radius whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @throws maliput::common::assertion_error When the represented number
///                                          is negative.
/// @warning This function will abort if preconditions are not met.
double RadiusFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);
  const double radius = std::strtod(argv[0], nullptr);
  MALIPUT_THROW_UNLESS(radius >= 0.);
  return radius;
}

/// @return An s coordinate position whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @throws maliput::common::assertion_error When the represented number
///                                          is negative.
/// @warning This function will abort if preconditions are not met.
double SFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);
  const double s = std::strtod(argv[0], nullptr);
  MALIPUT_THROW_UNLESS(s >= 0.);
  return s;
}

int Main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::SetUsageMessage(GetUsageMessage());
  if (argc < 3) {
    gflags::ShowUsageWithFlags(argv[0]);
    return 1;
  }
  const Command command = CommandsUsage().find(argv[2])->second;
  if (argc != command.num_arguments + 1) {
    gflags::ShowUsageWithFlags(argv[0]);
    return 1;
  }

  maliput::common::set_log_level(FLAGS_log_level);

  log()->debug("Loading road network using {} backend implementation...", FLAGS_maliput_backend);
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  // TODO(#): Extend this app to support other implementations than malidrive.
  if (maliput_implementation != MaliputImplementation::kMalidrive) {
    log()->error("maliput_query app only supports malidrive implementation for now.");
    return 1;
  }
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, FLAGS_linear_tolerance, FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file,
       FLAGS_phase_ring_book_file, FLAGS_intersection_book_file});
  log()->debug("RoadNetwork loaded successfully.");

  MALIPUT_DEMAND(rn != nullptr);

  auto rn_ptr = rn.get();
  RoadNetworkQuery query(&std::cout, const_cast<maliput::api::RoadNetwork*>(rn_ptr));
  std::cout << "Geometry Loaded" << std::endl;

  if (command.name.compare("FindRoadPositions") == 0) {
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[3]));
    const double radius = RadiusFromCLI(&(argv[6]));

    query.FindRoadPositions(inertial_position, radius);
  } else if (command.name.compare("ToRoadPosition") == 0) {
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[3]));

    query.ToRoadPosition(inertial_position);
  } else if (command.name.compare("ToLanePosition") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[3]));
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[4]));

    query.ToLanePosition(lane_id, inertial_position);
  } else if (command.name.compare("GetOrientation") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[3]));
    const maliput::api::LanePosition lane_position = LanePositionFromCLI(&(argv[4]));

    query.GetOrientation(lane_id, lane_position);
  } else if (command.name.compare("LaneToInertialPosition") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[3]));
    const maliput::api::LanePosition lane_position = LanePositionFromCLI(&(argv[4]));

    query.ToInertialPosition(lane_id, lane_position);
  } else if (command.name.compare("GetMaxSpeedLimit") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[3]));

    query.GetMaxSpeedLimit(lane_id);
  } else if (command.name.compare("GetDirectionUsage") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[3]));

    query.GetDirectionUsage(lane_id);
  } else if (command.name.compare("GetRightOfWay") == 0) {
    const maliput::api::LaneSRange lane_s_range = LaneSRangeFromCLI(&(argv[3]));

    query.GetRightOfWay(lane_s_range);
  } else if (command.name.compare("GetPhaseRightOfWay") == 0) {
    const maliput::api::rules::PhaseRing::Id phase_ring_id = PhaseRingIdFromCLI(&(argv[3]));
    const maliput::api::rules::Phase::Id phase_id = PhaseIdFromCLI(&(argv[4]));

    query.GetPhaseRightOfWay(phase_ring_id, phase_id);
  } else if (command.name.compare("GetDiscreteValueRules") == 0) {
    const maliput::api::LaneSRange lane_s_range = LaneSRangeFromCLI(&(argv[3]));

    query.GetDiscreteValueRule(lane_s_range);
  } else if (command.name.compare("GetRangeValueRules") == 0) {
    const maliput::api::LaneSRange lane_s_range = LaneSRangeFromCLI(&(argv[3]));

    query.GetRangeValueRule(lane_s_range);
  } else if (command.name.compare("GetLaneBounds") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[3]));
    const double s = SFromCLI(&(argv[4]));

    query.GetLaneBounds(lane_id, s);
  }

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
