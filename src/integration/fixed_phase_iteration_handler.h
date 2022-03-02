// Copyright 2022 Toyota Research Institute.
#pragma once

#include <maliput/api/road_network.h>
#include <maliput/common/maliput_copyable.h>

#include "integration/dynamic_environment_handler.h"
#include "integration/timer.h"

namespace maliput {
namespace integration {

/// DynamicEnvironmentHandler class implementation.
/// Each rule state is expected to last a fixed amount of time.
class FixedPhaseIterationHandler : public DynamicEnvironmentHandler {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(FixedPhaseIterationHandler)
  FixedPhaseIterationHandler() = delete;

  /// Constructs a FixedPhaseIterationHandler.
  /// @param timer Timer implementation pointer.
  /// @param road_network maliput::api::RoadNetwork pointer.
  /// @param phase_duration The duration of the rule's states in seconds.
  FixedPhaseIterationHandler(const Timer* timer, api::RoadNetwork* road_network, double phase_duration)
      : DynamicEnvironmentHandler(timer, road_network), phase_duration_(phase_duration) {
    MALIPUT_THROW_UNLESS(phase_duration > 0.);
  }

  ~FixedPhaseIterationHandler() override = default;

  void Update() override;

 private:
  const double phase_duration_{};
  double last_elapsed_time_{};
};

}  // namespace integration
}  // namespace maliput
