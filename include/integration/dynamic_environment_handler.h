// Copyright 2022 Toyota Research Institute.
#pragma once

#include <maliput/api/road_network.h>
#include <maliput/common/maliput_copyable.h>

#include "integration/timer.h"

namespace maliput {
namespace integration {

class DynamicEnvironmentHandler {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(DynamicEnvironmentHandler)
  DynamicEnvironmentHandler() = delete;
  DynamicEnvironmentHandler(const Timer* timer, api::RoadNetwork* road_network)
      : timer_(timer), road_network_(road_network) {
    MALIPUT_THROW_UNLESS(road_network_ != nullptr);
    MALIPUT_THROW_UNLESS(timer_ != nullptr);
  }
  virtual ~DynamicEnvironmentHandler() = default;

  virtual void Update() = 0;

 protected:
  const Timer* timer_{nullptr};
  api::RoadNetwork* road_network_{nullptr};
};

}  // namespace integration
}  // namespace maliput