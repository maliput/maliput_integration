// Copyright 2022 Toyota Research Institute.
#include "integration/fixed_phase_iteration_handler.h"

#include <maliput/base/manual_phase_provider.h>

namespace maliput {
namespace integration {

void FixedPhaseIterationHandler::Update() {
  if (!(timer_->Elapsed() - last_elapsed_time_ > phase_duration_)) {
    return;
  }
  last_elapsed_time_ = timer_->Elapsed();

  auto phase_provider = dynamic_cast<ManualPhaseProvider*>(road_network_->phase_provider());
  const auto phase_ring_book = road_network_->phase_ring_book();
  for (const auto& phase_ring_id : phase_ring_book->GetPhaseRings()) {
    const auto phase_ring = phase_ring_book->GetPhaseRing(phase_ring_id);
    const auto phase_provider_result = phase_provider->GetPhase(phase_ring_id);
    MALIPUT_THROW_UNLESS(phase_provider_result != std::nullopt);

    if (!phase_provider_result->next.has_value()) {
      continue;
    }

    const auto new_phase_id = phase_provider_result->next.value().state;
    const auto next_phases = phase_ring->GetNextPhases(new_phase_id);
    phase_provider->SetPhase(phase_ring_id, new_phase_id, next_phases.front().id, next_phases.front().duration_until);
  }
}

}  // namespace integration
}  // namespace maliput