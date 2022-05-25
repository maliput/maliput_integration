// BSD 3-Clause License
//
// Copyright (c) 2022, Woven Planet. All rights reserved.
// Copyright (c) 2022, Toyota Research Institute. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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