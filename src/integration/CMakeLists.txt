##############################################################################
# Sources
##############################################################################

add_library(integration
  chrono_timer.cc
  create_timer.cc
  fixed_phase_iteration_handler.cc
  tools.cc
)

add_library(maliput_integration::integration ALIAS integration)
set_target_properties(integration
  PROPERTIES
    OUTPUT_NAME maliput_integration_integration
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
  PRIVATE
    maliput_dragway::maliput_dragway
    maliput_malidrive::builder
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
    maliput_osm::builder
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
