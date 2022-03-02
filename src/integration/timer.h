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
  void Reset() { DoReset(); };

  /// @returns elapsed time in seconds.
  virtual double Elapsed() const { return DoElapsed(); }

 protected:
  Timer() = default;

 private:
  virtual void DoReset() = 0;
  virtual double DoElapsed() const = 0;
};

}  // namespace integration
}  // namespace maliput
