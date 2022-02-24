// Copyright 2022 Toyota Research Institute.
#pragma once

#include <memory>

#include <maliput/common/maliput_copyable.h>

#include "integration/timer.h"

namespace maliput {
namespace integration {

/// Forward Declaration.
class ChronoTimerImpl;

/// Timer implementation based on STL chrono library.
class ChronoTimer : public Timer {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(ChronoTimer)

  /// Constructs a ChronoTimer.
  ChronoTimer();

  /// Destructor.
  ~ChronoTimer() override;

  /// Documentation inherithed.
  void Reset() override;

  /// Documentation inherithed.
  double Elapsed() const override;

 private:
  std::unique_ptr<ChronoTimerImpl> impl_;
};

}  // namespace integration
}  // namespace maliput
