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
#pragma once

#include <optional>

#include <gflags/gflags.h>

#ifndef MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG

/// Declares FLAGS_log_level flag with all possible log level values.
#define MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG()                   \
  DEFINE_string(log_level, "unchanged",                               \
                "Sets the log output threshold; possible values are " \
                "'unchanged', "                                       \
                "'trace', "                                           \
                "'debug', "                                           \
                "'info', "                                            \
                "'warn', "                                            \
                "'err', "                                             \
                "'critical', "                                        \
                "'off'.")

#endif  // MALIPUT_APPLICATION_DEFINE_LOG_LEVEL_FLAG

#ifndef DRAGWAY_PROPERTIES_FLAGS

// By default, each lane is 3.7m (12 feet) wide, which is the standard used by
// the U.S. interstate highway system.
// By default, the shoulder width is 3 m (10 feet) wide, which is the standard
// used by the U.S. interstate highway system.
#define DRAGWAY_PROPERTIES_FLAGS()                                                                                 \
  DEFINE_int32(num_lanes, 2, "The number of lanes.");                                                              \
  DEFINE_double(length, 10, "The length of the dragway in meters.");                                               \
  DEFINE_double(lane_width, 3.7, "The width of each lane in meters.");                                             \
  DEFINE_double(shoulder_width, 3.0, "The width of the shoulders in meters. Both shoulders have the same width."); \
  DEFINE_double(maximum_height, 5.2, "The maximum modelled height above the road surface (meters).");

#endif  // DRAGWAY_PROPERTIES_FLAGS

#ifndef MULTILANE_PROPERTIES_FLAGS

#define MULTILANE_PROPERTIES_FLAGS() DEFINE_string(yaml_file, "", "yaml input file defining a multilane road geometry");

#endif  // MULTILANE_PROPERTIES_FLAGS

#ifndef COMMON_PROPERTIES_FLAGS
#define COMMON_PROPERTIES_FLAGS()                                                                 \
  DEFINE_double(linear_tolerance, 5e-2, "Linear tolerance used to load the map.");                \
  DEFINE_double(angular_tolerance, 1e-3, "Angular tolerance used to load the map.");              \
  DEFINE_string(rule_registry_file, "", "YAML file defining a Maliput rule registry");            \
  DEFINE_string(road_rule_book_file, "", "YAML file defining a Maliput road rule book");          \
  DEFINE_string(traffic_light_book_file, "", "YAML file defining a Maliput traffic lights book"); \
  DEFINE_string(phase_ring_book_file, "", "YAML file defining a Maliput phase ring book");        \
  DEFINE_string(intersection_book_file, "", "YAML file defining a Maliput intersection book");
#endif  // COMMON_PROPERTIES_FLAGS

#ifndef MALIDRIVE_PROPERTIES_FLAGS

#define MALIDRIVE_PROPERTIES_FLAGS()                                                                                   \
  DEFINE_string(xodr_file_path, "", "XODR file path.");                                                                \
  DEFINE_double(max_linear_tolerance, -1., "Maximum linear tolerance used to load the map.");                          \
  DEFINE_string(build_policy, "sequential", "Build policy, it could be `sequential` or `parallel`.");                  \
  DEFINE_int32(num_threads, 0, "Number of threads to create the Road Geometry.");                                      \
  DEFINE_string(simplification_policy, "none", "Geometries simplification policy, it could be `none` or `simplify`."); \
  DEFINE_string(                                                                                                       \
      standard_strictness_policy, "permissive",                                                                        \
      "OpenDrive standard strictness, it could be `permissive`, `allow_schema_errors`, `allow_semantic_errors` or "    \
      "`strict`. Union of policies are also allowed: 'allow_schema_errors|allow_semantic_errors'");                    \
  DEFINE_bool(omit_nondrivable_lanes, false, "If true, builder omits non-drivable lanes when building.");              \
  std::optional<double> GetLinearToleranceFlag() {                                                                     \
    return gflags::GetCommandLineFlagInfoOrDie("linear_tolerance").is_default                                          \
               ? std::nullopt                                                                                          \
               : std::make_optional<double>(FLAGS_linear_tolerance);                                                   \
  }                                                                                                                    \
  std::optional<double> GetMaxLinearToleranceFlag() {                                                                  \
    return gflags::GetCommandLineFlagInfoOrDie("max_linear_tolerance").is_default                                      \
               ? std::nullopt                                                                                          \
               : std::make_optional<double>(FLAGS_max_linear_tolerance);                                               \
  }
#endif  // MALIDRIVE_PROPERTIES_FLAGS

#ifndef MALIPUT_OSM_PROPERTIES_FLAGS

#define MALIPUT_OSM_PROPERTIES_FLAGS()           \
  DEFINE_string(osm_file, "", "OSM file path."); \
  DEFINE_string(origin, "{0., 0.}", "OSM map's origin lat/long coordinate.");
#endif  // MALIPUT_OSM_PROPERTIES_FLAGS
