// Copyright 2020 Toyota Research Institute
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

#ifndef MALIDRIVE_PROPERTIES_FLAGS

#define MALIDRIVE_PROPERTIES_FLAGS()                                                                                   \
  DEFINE_string(xodr_file_path, "", "XODR file path.");                                                                \
  DEFINE_double(linear_tolerance, 5e-2, "Linear tolerance used to load the map.");                                     \
  DEFINE_double(max_linear_tolerance, -1., "Maximum linear tolerance used to load the map.");                          \
  DEFINE_string(build_policy, "sequential", "Build policy, it could be `sequential` or `parallel`.");                  \
  DEFINE_int32(num_threads, 0, "Number of threads to create the Road Geometry.");                                      \
  DEFINE_string(simplification_policy, "none", "Geometries simplification policy, it could be `none` or `simplify`."); \
  DEFINE_string(                                                                                                       \
      standard_strictness_policy, "permissive",                                                                        \
      "OpenDrive standard strictness, it could be `permissive`, `allow_schema_errors`, `allow_semantic_errors` or "    \
      "`strict`. Union of policies are also allowed: 'allow_schema_errors|allow_semantic_errors'");                    \
  DEFINE_bool(omit_nondrivable_lanes, false, "If true, builder omits non-drivable lanes when building.");              \
  DEFINE_string(rule_registry_file, "", "YAML file defining a Maliput rule registry");                                 \
  DEFINE_string(road_rule_book_file, "", "YAML file defining a Maliput road rule book");                               \
  DEFINE_string(traffic_light_book_file, "", "YAML file defining a Maliput traffic lights book");                      \
  DEFINE_string(phase_ring_book_file, "", "YAML file defining a Maliput phase ring book");                             \
  DEFINE_string(intersection_book_file, "", "YAML file defining a Maliput intersection book");                         \
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
