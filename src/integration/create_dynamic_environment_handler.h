// Copyright 2022 Toyota Research Institute.
#pragma once

#include <memory>

#include "integration/dynamic_environment_handler.h"
#include "integration/fixed_phase_iteration_handler.h"

namespace maliput {
namespace integration {

/// Types of DynamicEnvironmentHandler implementations.
enum class DynamicEnvironmentHandlerType {
  kFixedPhaseIterationHandler,
};

/// Create Timer.
/// @param type A DynamicEnvironmentHandlerType.
/// @returns A DynamicEnvironmentHandler instance based on the selected implementation.
/// @tparam Args Arguments to be forwarded to the selected implementation.
template <typename... Args>
std::unique_ptr<DynamicEnvironmentHandler> CreateDynamicEnvironmentHandler(const DynamicEnvironmentHandlerType& type,
                                                                           Args&&... args) {
  switch (type) {
    case DynamicEnvironmentHandlerType::kFixedPhaseIterationHandler:
      return std::make_unique<maliput::integration::FixedPhaseIterationHandler>(std::forward<Args>(args)...);
      break;

    default:
      MALIPUT_THROW_MESSAGE("Unknown DynamicEnvironmentHandlerType value.");
      break;
  }
}

}  // namespace integration
}  // namespace maliput
