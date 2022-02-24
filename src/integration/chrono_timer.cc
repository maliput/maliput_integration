// Copyright 2022 Toyota Research Institute.
#include "integration/chrono_timer.h"

#include <chrono>

#include <maliput/common/maliput_copyable.h>

#include "integration/timer.h"

namespace maliput {
namespace integration {

class ChronoTimerImpl {
 public:
  ChronoTimerImpl() : start_(std::chrono::high_resolution_clock::now()) {}
  ~ChronoTimerImpl() = default;

  // Reset timer;
  void DoReset() { start_ = std::chrono::high_resolution_clock::now(); }

  // Returns elapsed time in seconds.
  double DoElapsed() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_)
               .count() /
           1000.;
  }
  std::chrono::high_resolution_clock::time_point start_{};
};

ChronoTimer::ChronoTimer() : Timer() { impl_ = std::make_unique<ChronoTimerImpl>(); }

ChronoTimer::~ChronoTimer() = default;

void ChronoTimer::Reset() { impl_->DoReset(); };

double ChronoTimer::Elapsed() const { return impl_->DoElapsed(); };

}  // namespace integration
}  // namespace maliput
