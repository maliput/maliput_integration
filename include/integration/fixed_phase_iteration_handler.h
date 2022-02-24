// Copyright 2022 Toyota Research Institute.
#pragma once

#include <maliput/api/road_network.h>
#include <maliput/common/maliput_copyable.h>

#include "integration/dynamic_environment_handler.h"
#include "integration/timer.h"

namespace maliput {
namespace integration {

class FixedPhaseIterationHandler : public DynamicEnvironmentHandler {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(FixedPhaseIterationHandler)
  FixedPhaseIterationHandler() = delete;
  FixedPhaseIterationHandler(const Timer* timer, api::RoadNetwork* road_network, double phase_duration)
      : DynamicEnvironmentHandler(timer, road_network), phase_duration_(phase_duration) {}
  ~FixedPhaseIterationHandler() override = default;

  void Update() override;

 private:
  const double phase_duration_{};
  double last_elapsed_time_{};
};

}  // namespace integration
}  // namespace maliput