// Copyright 2022 Toyota Research Institute.
#pragma once

#include <memory>

#include "integration/timer.h"

namespace maliput {
namespace integration {

/// Timer implementations.
enum class TimerType {
  kChronoTimer,
};

/// Create Timer.
/// @param type A TimerType.
/// @returns A Timer instance based on the selected implementation.
std::unique_ptr<Timer> CreateTimer(const TimerType& type);

}  // namespace integration
}  // namespace maliput
