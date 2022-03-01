// Copyright 2022 Toyota Research Institute.
#pragma once

#include <chrono>
#include <memory>

#include <maliput/common/maliput_copyable.h>

#include "integration/timer.h"

namespace maliput {
namespace integration {

/// Timer implementation based on STL chrono library.
class ChronoTimer : public Timer {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(ChronoTimer)

  /// Constructs a ChronoTimer.
  ChronoTimer();

  /// Destructor.
  ~ChronoTimer() override = default;

 private:
  void DoReset() override;
  double DoElapsed() const override;

  std::chrono::high_resolution_clock::time_point start_{};
};

}  // namespace integration
}  // namespace maliput
