// Copyright 2022 Toyota Research Institute.
#include "integration/chrono_timer.h"

#include <maliput/common/maliput_copyable.h>

namespace maliput {
namespace integration {

ChronoTimer::ChronoTimer() : Timer(), start_(std::chrono::high_resolution_clock::now()) {}

void ChronoTimer::DoReset() { start_ = std::chrono::high_resolution_clock::now(); }

double ChronoTimer::DoElapsed() const {
  return static_cast<double>(
             std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_)
                 .count()) /
         1000.;
}

}  // namespace integration
}  // namespace maliput
