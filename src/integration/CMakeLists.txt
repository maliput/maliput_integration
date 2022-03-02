##############################################################################
# Sources
##############################################################################

add_library(integration
  chrono_timer.cc
  create_timer.cc
  fixed_phase_iteration_handler.cc
  tools.cc
)

target_include_directories(integration
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(integration
  PUBLIC
    maliput::api
    maliput::base
    maliput::common
    maliput_dragway::maliput_dragway
    maliput_malidrive::builder
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
    yaml-cpp
)

##############################################################################
# Export
##############################################################################

install(
  TARGETS integration
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_libraries(integration)
