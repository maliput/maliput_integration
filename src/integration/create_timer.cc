// Copyright 2022 Toyota Research Institute.
#include "integration/create_timer.h"

#include <maliput/common/maliput_throw.h>

#include "integration/chrono_timer.h"

namespace maliput {
namespace integration {

std::unique_ptr<Timer> CreateTimer(const TimerType& type) {
  switch (type) {
    case TimerType::kChronoTimer:
      return std::make_unique<maliput::integration::ChronoTimer>();
      break;

    default:
      MALIPUT_THROW_MESSAGE("Not identified timer type.");
      break;
  }
}

}  // namespace integration
}  // namespace maliput
