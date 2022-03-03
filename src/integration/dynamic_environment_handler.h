// Copyright 2022 Toyota Research Institute.
#pragma once

#include <maliput/api/road_network.h>
#include <maliput/common/maliput_copyable.h>

#include "integration/timer.h"

namespace maliput {
namespace integration {

/// Abstract API for managing the rules dynamic states of a maliput::api::RoadNetwork.
/// The states are expected to change based on time.
class DynamicEnvironmentHandler {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(DynamicEnvironmentHandler)
  DynamicEnvironmentHandler() = delete;

  virtual ~DynamicEnvironmentHandler() = default;

  /// Updates the rule's states.
  virtual void Update() = 0;

 protected:
  /// Creates DynamicEnvironmentHandler
  /// @param timer Timer implementation pointer.
  /// @param road_network maliput::api::RoadNetwork pointer.
  DynamicEnvironmentHandler(const Timer* timer, api::RoadNetwork* road_network)
      : timer_(timer), road_network_(road_network) {
    MALIPUT_THROW_UNLESS(road_network_ != nullptr);
    MALIPUT_THROW_UNLESS(timer_ != nullptr);
  }

  const Timer* timer_{nullptr};
  api::RoadNetwork* road_network_{nullptr};
};

}  // namespace integration
}  // namespace maliput
