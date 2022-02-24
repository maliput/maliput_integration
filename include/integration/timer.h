// Copyright 2022 Toyota Research Institute.
#pragma once

#include <maliput/common/maliput_copyable.h>

namespace maliput {
namespace integration {

/// Simple Timer API.
class Timer {
 public:
  MALIPUT_NO_COPY_NO_MOVE_NO_ASSIGN(Timer)
  virtual ~Timer() = default;

  /// Reset Timer.
  virtual void Reset() = 0;

  /// @returns elapsed time in seconds.
  virtual double Elapsed() const = 0;

 protected:
  Timer() = default;
};

}  // namespace integration
}  // namespace maliput
