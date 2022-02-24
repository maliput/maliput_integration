##############################################################################
# Sources
##############################################################################
add_executable(maliput_to_obj
  maliput_to_obj.cc
)

target_link_libraries(maliput_to_obj
    gflags
    integration
    maliput::common
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
)

add_executable(maliput_to_string
  maliput_to_string.cc
)

target_link_libraries(maliput_to_string
    gflags
    integration
    maliput::common
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
)

add_executable(maliput_query
  maliput_query.cc
)

target_link_libraries(maliput_query
    gflags
    integration
    maliput::common
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
)

add_executable(maliput_derive_lane_s_routes
  maliput_derive_lane_s_routes.cc
)

target_link_libraries(maliput_derive_lane_s_routes
    gflags
    integration
    maliput::common
    maliput::routing
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
)

add_executable(maliput_measure_load_time
  maliput_measure_load_time.cc
)

target_link_libraries(maliput_measure_load_time
    gflags
    integration
    maliput::common
    maliput::routing
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
)

add_executable(maliput_to_string_with_plugin
  maliput_to_string_with_plugin.cc
)

target_link_libraries(maliput_to_string_with_plugin
  gflags
  maliput::api
  maliput::plugin
  maliput::common
  maliput::utilities
)

add_executable(maliput_dynamic_environment
  maliput_dynamic_environment.cc
)

target_link_libraries(maliput_dynamic_environment
  gflags
  integration
  maliput::api
  maliput::plugin
  maliput::common
  maliput::utilities
)

##############################################################################
# Install
##############################################################################

install(
  TARGETS
    maliput_derive_lane_s_routes
    maliput_dynamic_environment
    maliput_measure_load_time
    maliput_query
    maliput_to_obj
    maliput_to_string
    maliput_to_string_with_plugin
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

add_subdirectory(python)
