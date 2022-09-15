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

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <maliput/common/logger.h>
#include <maliput/common/maliput_abort.h>
#include <maliput/math/bounding_box.h>
#include <maliput/math/overlapping_type.h>
#include <maliput_malidrive/constants.h>
#include <maliput_malidrive/utility/file_tools.h>
#include <maliput_object/api/object.h>
#include <maliput_object/base/manual_object_book.h>
#include <maliput_object/base/simple_object_query.h>

#include "integration/tools.h"
#include "maliput_gflags.h"

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
        "FindRoadPositions x y z r",
        {"Obtains, for all Lanes whose segment regions include points",
         "that are within a radius r of an (x, y, z) InertialPosition i.e. in",
         "the world frame, the RoadPosition of the point in the Lane manifold",
         "which is closest to that InertialPosition."},
        5}},
      {"ToRoadPosition",
       {"ToRoadPosition",
        "ToRoadPosition x y z",
        {"Obtains the RoadPosition of the point in the RoadGeometry manifold",
         "which is, in the world frame, closest to an (x, y, z) InertialPosition."},
        4}},
      {"ToLanePosition",
       {"ToLanePosition",
        "ToLanePosition lane_id x y z",
        {"Obtains the LanePosition in a Lane, identified by lane_id, that is",
         "closest, in the world frame, to an (x, y, z) InertialPosition."},
        5}},
      {"ToSegmentPosition",
       {"ToSegmentPosition",
        "ToSegmentPosition lane_id x y z",
        {"Obtains the LanePosition in a Lane within the segment's boundaries, identified by lane_id, that is",
         "closest, in the world frame, to an (x, y, z) InertialPosition."},
        5}},
      {"GetOrientation",
       {"GetOrientation",
        "GetOrientation lane_id s r h",
        {"Obtains the orientation in a Lane, identified by lane_id, that is",
         "closest, in the world frame, to an (s, r, h) LanePosition."},
        5}},
      {"ToInertialPosition",
       {"ToInertialPosition",
        "ToInertialPosition lane_id s r h",
        {"Obtains the InertialPosition for an (s, r, h) LanePosition in a Lane,", "identified by lane_id."},
        5}},
      {"GetMaxSpeedLimit",
       {"GetMaxSpeedLimit",
        "GetMaxSpeedLimit lane_id",
        {"Obtains the maximum SpeedLimitRule for a Lane identified by lane_id.",
         "Rules are defined on the RoadRuleBook as loaded from a "
         "--road_rule_book_file."},
        2}},
      {"GetDirectionUsage",
       {"GetDirectionUsage",
        "GetDirectionUsage lane_id",
        {"Obtains all DirectionUsageRules for a Lane identified by lane_id.",
         "Rules are defined on the RoadRuleBook as loaded from a "
         "--road_rule_book_file."},
        2}},
      {"GetRightOfWay",
       {"GetRightOfWay",
        "GetRightOfWay lane_id start_s end_s",
        {"Obtains all RightOfWayRules for a region [start_s, end_s] of a Lane,",
         "identified by lane_id. Rules are defined on the RoadRuleBook as loaded", "from a --road_rule_book_file."},
        4}},
      {"GetPhaseRightOfWay",
       {"GetPhaseRightOfWay",
        "GetPhaseRightOfWay phase_ring_id phase_id",
        {"Obtains the state of RightOfWayRules for a Phase identified by"
         " phase_id",
         "in a PhaseRing identified by phase_ring_id.",
         "Rules are defined on the RoadRuleBook as loaded from "
         "a --road_rule_book_file.",
         "Phases are defined on the PhaseRingBook as loaded from "
         "a --phase_ring_book_file."},
        3}},
      {"GetDiscreteValueRules",
       {"GetDiscreteValueRules",
        "GetDiscreteValueRules lane_id start_s end_s",
        {"Obtains all DiscreteValueRule for a region [start_s, end_s] of a Lane,",
         "identified by lane_id. Rules are defined on the RoadRuleBook as loaded",
         "from a --road_rule_book_file or the xodr itself."},
        4}},
      {"GetRangeValueRules",
       {"GetRangeValueRules",
        "GetRangeValueRules lane_id start_s end_s",
        {"Obtains all RangeValueRules for a region [start_s, end_s] of a Lane,",
         "identified by lane_id. Rules are defined on the RoadRuleBook as loaded",
         "from a --road_rule_book_file or the xodr itself."},
        4}},
      {"GetLaneBounds",
       {"GetLaneBounds",
        "GetLaneBounds lane_id s",
        {"Obtains the segment and lane bounds of lane_id at s position. Return strings would be: ",
         "[segment_bounds.min, lane_bounds.min, lane_bounds.max, segment_bounds.max]."},
        3}},
      {"GetLaneLength", {"GetLaneLength", "GetLaneLength lane_id", {"Obtains the length of the lane."}, 2}},
      {"GetSegmentBounds",
       {"GetSegmentBounds",
        "GetSegmentBounds segment_id s",
        {"Obtains the segment bounds of segment_id at s position. Return strings would be: ",
         "[segment_bounds.min, segment_bounds.max]."},
        3}},
      {"GetNumberOfLanes",
       {"GetNumberOfLanes", "GetNumberOfLanes", {"Obtains number of lanes in the RoadGeometry."}, 1}},

      {"FindOverlappingLanesIn",
       {"FindOverlappingLanesIn",
        "FindOverlappingLanesIn overlapping_type box_length box_width box_height x y z roll pitch yaw",
        {"Obtains the Lanes that overlap with a Bounding Box of size [box_length, box_width, box_height]",
         "with a pose [x, y, z, roll, pitch, yaw] according to the selected [overlapping_type]:",
         " - intersected: Returns lanes that intersect the bounding box.",
         " - disjointed: Returns lanes that don't intersect the bounding box.",
         " - contained: Returns the lanes that are contained within the bounding box."},
        11}},
      {"Route",
       {"Route",
        "Route box_length_1 box_width_1 box_height_1 x_1 y_1 z_1 roll_1 pitch_1 yaw_1 box_length_2 box_width_2 "
        "box_height_2 x_2 y_2 z_2 roll_2 pitch_2 yaw_2",
        {"Obtains the route from a bounding box ",
         "of size [box_length_1, box_width_1, box_height_1] and pose [x_1, y_1, z_1, roll_1, pitch_1, yaw_1] ",
         "to a bounding box of size [box_length_2, box_width_2, box_height_2] ",
         "and pose [x_2, y_2, z_2, roll_2, pitch_2, yaw_2]"},
        19}},
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
  ss << "  maliput_query -- <command> <arg1> <arg2> ... <argN> " << std::endl << std::endl;
  ss << "  Supported commands:" << std::endl;
  const std::map<const std::string, const Command> command_usage = CommandsUsage();
  for (auto it = command_usage.begin(); it != command_usage.end(); ++it) {
    ss << "    " << it->second.usage << std::endl << std::endl;
    for (const std::string& line : it->second.description) {
      ss << "        " << line << std::endl;
    }
    ss << std::endl;
  }
  ss << "  Examples of use: " << std::endl;
  ss << "    $ maliput_query --maliput_backend=malidrive --xodr_file_path=TShapeRoad.xodr -- GetLaneLength 1_0_1"
     << std::endl;
  ss << "    $ maliput_query --maliput_backend=malidrive --xodr_file_path=TShapeRoad.xodr -- ToRoadPosition 0.0 -1.5 "
        "2.0"
     << std::endl
     << std::endl;

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

    object_book_ = std::make_unique<maliput::object::ManualObjectBook<maliput::math::Vector3>>();
    object_query_ = std::make_unique<maliput::object::SimpleObjectQuery>(rn_, object_book_.get());
  }

  /// Redirects `inertial_position` and `radius` to RoadGeometry::FindRoadPosition().
  void FindRoadPositions(const maliput::api::InertialPosition& inertial_position, double radius) {
    const auto start = std::chrono::high_resolution_clock::now();
    const std::vector<maliput::api::RoadPositionResult> results =
        rn_->road_geometry()->FindRoadPositions(inertial_position, radius);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "FindRoadPositions(inertial_position:" << inertial_position << ", radius: " << radius << ")"
            << std::endl;
    for (const maliput::api::RoadPositionResult& result : results) {
      (*out_) << "              : Result: " << result << std::endl;
    }
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Redirects `lane_position` to `lane_id`'s Lane::ToInertialPosition().
  void ToInertialPosition(const maliput::api::LaneId& lane_id, const maliput::api::LanePosition& lane_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);

    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::InertialPosition inertial_position = lane->ToInertialPosition(lane_position);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "(" << lane_id.string() << ")->ToInertialPosition(lane_position: " << lane_position << ")" << std::endl;
    (*out_) << "              : Result: inertial_position:" << inertial_position << std::endl;

    const maliput::api::RoadPositionResult result =
        rn_->road_geometry()->ToRoadPosition(inertial_position, std::nullopt);

    (*out_) << "              : Result round_trip inertial_position" << result.nearest_position
            << ", with distance: " << result.distance << std::endl;
    (*out_) << "              : RoadPosition: " << result.road_position << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Redirects `inertial_position` to `lane_id`'s Lane::ToLanePosition().
  void ToLanePosition(const maliput::api::LaneId& lane_id, const maliput::api::InertialPosition& inertial_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::LanePositionResult lane_position_result = lane->ToLanePosition(inertial_position);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "(" << lane_id.string() << ")->ToLanePosition(inertial_position: " << inertial_position << ")"
            << std::endl;
    (*out_) << "              : Result: lane_pos:" << lane_position_result.lane_position
            << ", nearest_pos: " << lane_position_result.nearest_position
            << ", with distance: " << lane_position_result.distance << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Redirects `inertial_position` to `lane_id`'s Lane::ToSegmentPosition().
  void ToSegmentPosition(const maliput::api::LaneId& lane_id, const maliput::api::InertialPosition& inertial_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::LanePositionResult lane_position_result = lane->ToSegmentPosition(inertial_position);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "(" << lane_id.string() << ")->ToSegmentPosition(inertial_position: " << inertial_position << ")"
            << std::endl;
    (*out_) << "              : Result: lane_pos:" << lane_position_result.lane_position
            << ", nearest_pos: " << lane_position_result.nearest_position
            << ", with distance: " << lane_position_result.distance << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Redirects `lane_position` to `lane_id`'s Lane::GetOrientation().
  void GetOrientation(const maliput::api::LaneId& lane_id, const maliput::api::LanePosition& lane_position) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);

    if (lane == nullptr) {
      (*out_) << "              : Result: Could not find lane. " << std::endl;
      return;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::Rotation rotation = lane->GetOrientation(lane_position);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "(" << lane_id.string() << ")->GetOrientation(lane_position: " << lane_position << ")" << std::endl;
    (*out_) << "              : Result: orientation:" << rotation << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Redirects `inertial_position` to RoadGeometry::ToRoadPosition().
  void ToRoadPosition(const maliput::api::InertialPosition& inertial_position) {
    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::RoadPositionResult result = rn_->road_geometry()->ToRoadPosition(inertial_position);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "ToRoadPosition(inertial_position: " << inertial_position << ")" << std::endl;
    (*out_) << "              : Result: nearest_pos:" << result.nearest_position
            << " with distance: " << result.distance << std::endl;
    (*out_) << "                RoadPosition: " << result.road_position << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Looks for all the maximum speed limits allowed at `lane_id`.
  void GetMaxSpeedLimit(const maliput::api::LaneId& lane_id) {
    const auto start = std::chrono::high_resolution_clock::now();
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
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Looks for all the direction usages at `lane_id`.
  void GetDirectionUsage(const maliput::api::LaneId& lane_id) {
    const auto start = std::chrono::high_resolution_clock::now();
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
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  /// Gets all right-of-way rules for the given `lane_s_range`.
  void GetRightOfWay(const maliput::api::LaneSRange& lane_s_range) {
    const auto start = std::chrono::high_resolution_clock::now();
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
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }
#pragma GCC diagnostic pop

  /// Gets all discrete-value-rules rules for the given `lane_s_range`.
  void GetDiscreteValueRule(const maliput::api::LaneSRange& lane_s_range) {
    const auto start = std::chrono::high_resolution_clock::now();
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
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets all range-value-rules rules for the given `lane_s_range`.
  void GetRangeValueRule(const maliput::api::LaneSRange& lane_s_range) {
    const auto start = std::chrono::high_resolution_clock::now();
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
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets all right-of-way rules' states for a given phase in a given phase
  /// ring.
  void GetPhaseRightOfWay(const maliput::api::rules::PhaseRing::Id& phase_ring_id,
                          const maliput::api::rules::Phase::Id& phase_id) {
    const auto start = std::chrono::high_resolution_clock::now();
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    for (const auto& rule_id_to_rule_state_id : phase.rule_states()) {
      const maliput::api::rules::RightOfWayRule rule = road_rule_book->GetRule(rule_id_to_rule_state_id.first);
      const maliput::api::rules::RightOfWayRule::State& rule_state = rule.states().at(rule_id_to_rule_state_id.second);
      (*out_) << "    Rule(id: " << rule.id().string() << ", zone: " << rule.zone() << ", zone-type: '"
              << rule.zone_type() << "'"
              << ", current_state: " << rule_state << ", static: " << (rule.is_static() ? "yes" : "no") << ")"
              << std::endl
              << std::endl;
    }
#pragma GCC diagnostic pop
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets a lane boundaries for `lane_id` at `s`.
  void GetLaneBounds(const maliput::api::LaneId& lane_id, double s) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    if (lane == nullptr) {
      std::cerr << " Could not find lane. " << std::endl;
      return;
    }
    const maliput::api::RBounds segment_bounds = lane->segment_bounds(s);

    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::RBounds lane_bounds = lane->lane_bounds(s);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "Lateral boundaries for  " << lane_id.string() << ":" << std::endl
            << "    [" << segment_bounds.min() << "; " << lane_bounds.min() << "; " << lane_bounds.max() << "; "
            << segment_bounds.max() << "]" << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets a segment boundary for `segment_id` at `s`.
  void GetSegmentBounds(const maliput::api::SegmentId& segment_id, double s) {
    const maliput::api::Segment* segment = rn_->road_geometry()->ById().GetSegment(segment_id);
    if (segment == nullptr) {
      std::cerr << " Could not find segment. " << std::endl;
      return;
    }
    // Segments bounds are computed from a Lane.
    const auto start = std::chrono::high_resolution_clock::now();
    const maliput::api::RBounds segment_bounds = segment->lane(0)->segment_bounds(s);
    const auto end = std::chrono::high_resolution_clock::now();

    (*out_) << "Segment boundaries for segment " << segment_id.string() << ":" << std::endl
            << "    [" << segment_bounds.min() << "; " << segment_bounds.max() << "]" << std::endl;

    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets the lane length for `lane_id`.
  void GetLaneLength(const maliput::api::LaneId& lane_id) {
    const maliput::api::Lane* lane = rn_->road_geometry()->ById().GetLane(lane_id);
    const auto start = std::chrono::high_resolution_clock::now();
    const double length = lane->length();
    const auto end = std::chrono::high_resolution_clock::now();
    if (lane == nullptr) {
      std::cerr << " Could not find lane. " << std::endl;
      return;
    }
    (*out_) << "Lane length for  " << lane_id.string() << ":    [" << std::to_string(length) << " m]" << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets number of lanes in the RoadGeometry.
  void GetNumberOfLanes() {
    const auto start = std::chrono::high_resolution_clock::now();
    const std::size_t num_lanes{rn_->road_geometry()->ById().GetLanes().size()};
    const auto end = std::chrono::high_resolution_clock::now();
    (*out_) << "Number of lanes in the RoadGeometry: " << num_lanes << std::endl;
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets all the Lanes (according to the overlapping type) in respect to a BoundingRegion
  void FindOverlappingLanesIn(const maliput::object::api::Object<maliput::math::Vector3>* bounding_object_ptr,
                              const maliput::math::OverlappingType overlapping_type) {
    static const std::map<maliput::math::OverlappingType, std::string> overlapping_type_to_string{
        {maliput::math::OverlappingType::kDisjointed, "disjointed"},
        {maliput::math::OverlappingType::kIntersected, "intersected"},
        {maliput::math::OverlappingType::kContained, "contained"}};
    const auto start = std::chrono::high_resolution_clock::now();
    const std::vector<const maliput::api::Lane*> overlapping_lanes =
        object_query_->FindOverlappingLanesIn(bounding_object_ptr, overlapping_type);
    const auto end = std::chrono::high_resolution_clock::now();
    (*out_) << "The " << overlapping_type_to_string.at(overlapping_type)
            << " overlapping lanes for the object: " << std::endl;
    PrintObjectProperties(bounding_object_ptr);
    (*out_) << "Are the following: " << std::endl;
    for (const auto& lane : overlapping_lanes) {
      (*out_) << "  Lane Id: " << lane->id() << std::endl;
    };
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// Gets all the lanes needed to get from the position of an Object to the position of another Object
  void Route(const maliput::object::api::Object<maliput::math::Vector3>* bounding_object_1_ptr,
             const maliput::object::api::Object<maliput::math::Vector3>* bounding_object_2_ptr) {
    const auto start = std::chrono::high_resolution_clock::now();
    const std::optional<const maliput::api::LaneSRoute> route =
        object_query_->Route(bounding_object_1_ptr, bounding_object_2_ptr);
    const auto end = std::chrono::high_resolution_clock::now();
    if (route.has_value()) {
      (*out_) << "The Route from the object: " << std::endl;
      PrintObjectProperties(bounding_object_1_ptr);
      (*out_) << "to the object: " << std::endl;
      PrintObjectProperties(bounding_object_2_ptr);
      (*out_) << "is the following: \n" << route.value() << std::endl;
    } else {
      (*out_) << "There is no Route between object: " << std::endl;
      PrintObjectProperties(bounding_object_1_ptr);
      (*out_) << "and object: " << std::endl;
      PrintObjectProperties(bounding_object_2_ptr);
    };
    const std::chrono::duration<double> duration = (end - start);
    PrintQueryTime(duration.count());
  }

  /// @return the object_book_ variable.
  maliput::object::ManualObjectBook<maliput::math::Vector3>* GetManualObjectBook() { return object_book_.get(); }

 private:
  // Prints "Elapsed Query Time: < @p sec >".
  static void PrintQueryTime(double sec) { std::cout << "Elapsed Query Time: " << sec << " s" << std::endl; }

  // Prints the Object properties (size, position and orientation).
  static void PrintObjectProperties(const maliput::object::api::Object<maliput::math::Vector3>* object_ptr) {
    // TODO Add size and orientation from the bounding region to the print.
    // std::cout << "  Object Id: "<< object_ptr->id()<<std::endl;
    const maliput::math::BoundingBox* bounding_box_ptr =
        dynamic_cast<const maliput::math::BoundingBox*>(&(object_ptr->bounding_region()));
    std::cout << "  Size:        " << bounding_box_ptr->box_size() << std::endl;
    std::cout << "  Position:    " << object_ptr->position() << std::endl;
    std::cout << "  Orientation: " << bounding_box_ptr->get_orientation().vector() << std::endl;
  }

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
  std::unique_ptr<maliput::object::ManualObjectBook<maliput::math::Vector3>> object_book_;
  std::unique_ptr<maliput::object::SimpleObjectQuery> object_query_;
};

/// @return A LaneId whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::LaneId LaneIdFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);
  return maliput::api::LaneId(std::string(*argv));
}

/// @return An OverlappingType according to the selected by `*argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::math::OverlappingType OverlappingTypeFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);

  static const std::map<std::string, maliput::math::OverlappingType> string_to_overlapping_type{
      {"disjointed", maliput::math::OverlappingType::kDisjointed},
      {"intersected", maliput::math::OverlappingType::kIntersected},
      {"contained", maliput::math::OverlappingType::kContained}};

  const std::string overlapping_type_str = *argv;
  MALIPUT_DEMAND(string_to_overlapping_type.find(overlapping_type_str) != string_to_overlapping_type.end());

  return string_to_overlapping_type.at(overlapping_type_str);
}

/// @return A maliput Object with the selected id.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
std::unique_ptr<maliput::object::api::Object<maliput::math::Vector3>> ObjectFromCLI(const std::string id, char** argv) {
  MALIPUT_DEMAND(argv != nullptr);

  const double s_x = std::strtod(argv[0], nullptr);
  const double s_y = std::strtod(argv[1], nullptr);
  const double s_z = std::strtod(argv[2], nullptr);
  MALIPUT_DEMAND(s_x >= 0 && s_y >= 0 && s_z >= 0);
  const maliput::math::Vector3 size{s_x, s_y, s_z};

  const double p_x = std::strtod(argv[3], nullptr);
  const double p_y = std::strtod(argv[4], nullptr);
  const double p_z = std::strtod(argv[5], nullptr);
  const maliput::math::Vector3 position{p_x, p_y, p_z};

  const double roll = std::strtod(argv[6], nullptr);
  const double pitch = std::strtod(argv[7], nullptr);
  const double yaw = std::strtod(argv[8], nullptr);
  const maliput::math::RollPitchYaw orientation{roll, pitch, yaw};

  return std::make_unique<maliput::object::api::Object<maliput::math::Vector3>>(
      maliput::object::api::Object<maliput::math::Vector3>::Id{id}, std::map<std::string, std::string>{},
      std::make_unique<maliput::math::BoundingBox>(position, size, orientation, 1e-6));
}

/// @return A SegmentId whose string representation is `*argv`.
/// @pre `argv` is not nullptr.
/// @warning This function will abort if preconditions are not met.
maliput::api::SegmentId SegmentIdFromCLI(char** argv) {
  MALIPUT_DEMAND(argv != nullptr);
  return maliput::api::SegmentId(std::string(*argv));
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
  gflags::SetUsageMessage(GetUsageMessage());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (argc < 2) {
    maliput::log()->error("Not valid command provided.\nRun 'maliput_query --help' for help.\n");
    return 1;
  }
  const auto commands_usage = CommandsUsage();
  const auto command_it = commands_usage.find(argv[1]);
  if (command_it == commands_usage.end()) {
    maliput::log()->error("Not valid command provided: {}\nRun 'maliput_query --help' for help.\n", argv[1]);
    return 1;
  }
  const Command command = command_it->second;
  if (argc != command.num_arguments + 1) {
    maliput::log()->error("Missing arguments for command: {}\nRun 'maliput_query --help' for help.\n", command.usage);
    return 1;
  }

  maliput::common::set_log_level(FLAGS_log_level);

  log()->info("Loading road network using {} backend implementation...", FLAGS_maliput_backend);
  const MaliputImplementation maliput_implementation{StringToMaliputImplementation(FLAGS_maliput_backend)};
  auto rn = LoadRoadNetwork(
      maliput_implementation,
      {FLAGS_num_lanes, FLAGS_length, FLAGS_lane_width, FLAGS_shoulder_width, FLAGS_maximum_height}, {FLAGS_yaml_file},
      {FLAGS_xodr_file_path, GetLinearToleranceFlag(), GetMaxLinearToleranceFlag(), FLAGS_build_policy,
       FLAGS_num_threads, FLAGS_simplification_policy, FLAGS_standard_strictness_policy, FLAGS_omit_nondrivable_lanes,
       FLAGS_rule_registry_file, FLAGS_road_rule_book_file, FLAGS_traffic_light_book_file, FLAGS_phase_ring_book_file,
       FLAGS_intersection_book_file});
  MALIPUT_DEMAND(rn != nullptr);
  log()->info("RoadNetwork loaded successfully.");

  auto rn_ptr = rn.get();
  RoadNetworkQuery query(&std::cout, const_cast<maliput::api::RoadNetwork*>(rn_ptr));

  if (command.name.compare("FindRoadPositions") == 0) {
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[2]));
    const double radius = RadiusFromCLI(&(argv[5]));

    query.FindRoadPositions(inertial_position, radius);
  } else if (command.name.compare("ToRoadPosition") == 0) {
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[2]));

    query.ToRoadPosition(inertial_position);
  } else if (command.name.compare("ToLanePosition") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[3]));

    query.ToLanePosition(lane_id, inertial_position);
  } else if (command.name.compare("ToSegmentPosition") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));
    const maliput::api::InertialPosition inertial_position = InertialPositionFromCLI(&(argv[3]));

    query.ToSegmentPosition(lane_id, inertial_position);
  } else if (command.name.compare("GetOrientation") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));
    const maliput::api::LanePosition lane_position = LanePositionFromCLI(&(argv[3]));

    query.GetOrientation(lane_id, lane_position);
  } else if (command.name.compare("ToInertialPosition") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));
    const maliput::api::LanePosition lane_position = LanePositionFromCLI(&(argv[3]));

    query.ToInertialPosition(lane_id, lane_position);
  } else if (command.name.compare("GetMaxSpeedLimit") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));

    query.GetMaxSpeedLimit(lane_id);
  } else if (command.name.compare("GetDirectionUsage") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));

    query.GetDirectionUsage(lane_id);
  } else if (command.name.compare("GetRightOfWay") == 0) {
    const maliput::api::LaneSRange lane_s_range = LaneSRangeFromCLI(&(argv[2]));

    query.GetRightOfWay(lane_s_range);
  } else if (command.name.compare("GetPhaseRightOfWay") == 0) {
    const maliput::api::rules::PhaseRing::Id phase_ring_id = PhaseRingIdFromCLI(&(argv[2]));
    const maliput::api::rules::Phase::Id phase_id = PhaseIdFromCLI(&(argv[3]));

    query.GetPhaseRightOfWay(phase_ring_id, phase_id);
  } else if (command.name.compare("GetDiscreteValueRules") == 0) {
    const maliput::api::LaneSRange lane_s_range = LaneSRangeFromCLI(&(argv[2]));

    query.GetDiscreteValueRule(lane_s_range);
  } else if (command.name.compare("GetRangeValueRules") == 0) {
    const maliput::api::LaneSRange lane_s_range = LaneSRangeFromCLI(&(argv[2]));

    query.GetRangeValueRule(lane_s_range);
  } else if (command.name.compare("GetLaneBounds") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));
    const double s = SFromCLI(&(argv[3]));

    query.GetLaneBounds(lane_id, s);
  } else if (command.name.compare("GetSegmentBounds") == 0) {
    const maliput::api::SegmentId segment_id = SegmentIdFromCLI(&(argv[2]));
    const double s = SFromCLI(&(argv[3]));

    query.GetSegmentBounds(segment_id, s);
  } else if (command.name.compare("GetLaneLength") == 0) {
    const maliput::api::LaneId lane_id = LaneIdFromCLI(&(argv[2]));

    query.GetLaneLength(lane_id);
  } else if (command.name.compare("GetNumberOfLanes") == 0) {
    query.GetNumberOfLanes();

  } else if (command.name.compare("FindOverlappingLanesIn") == 0) {
    const maliput::math::OverlappingType overlapping_type = OverlappingTypeFromCLI(&(argv[2]));
    std::unique_ptr<maliput::object::api::Object<maliput::math::Vector3>> bounding_object =
        ObjectFromCLI(std::string{"Box_1"}, &(argv[3]));
    const maliput::object::api::Object<maliput::math::Vector3>* bounding_object_ptr = bounding_object.get();
    query.GetManualObjectBook()->AddObject(std::move(bounding_object));
    query.FindOverlappingLanesIn(bounding_object_ptr, overlapping_type);

  } else if (command.name.compare("Route") == 0) {
    std::unique_ptr<maliput::object::api::Object<maliput::math::Vector3>> bounding_object_1 =
        ObjectFromCLI(std::string{"Box_1"}, &(argv[3]));
    std::unique_ptr<maliput::object::api::Object<maliput::math::Vector3>> bounding_object_2 =
        ObjectFromCLI(std::string{"Box_2"}, &(argv[3]));
    const maliput::object::api::Object<maliput::math::Vector3>* bounding_object_ptr_1 = bounding_object_1.get();
    const maliput::object::api::Object<maliput::math::Vector3>* bounding_object_ptr_2 = bounding_object_2.get();
    query.GetManualObjectBook()->AddObject(std::move(bounding_object_1));
    query.GetManualObjectBook()->AddObject(std::move(bounding_object_2));
    query.Route(bounding_object_ptr_1, bounding_object_ptr_2);
  }

  return 0;
}

}  // namespace
}  // namespace integration
}  // namespace maliput

int main(int argc, char* argv[]) { return maliput::integration::Main(argc, argv); }
