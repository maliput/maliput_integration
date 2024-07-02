##############################################################################
# Sources
##############################################################################
add_executable(maliput_to_obj
  maliput_to_obj.cc
)

target_link_libraries(maliput_to_obj
    gflags
    maliput::common
    maliput::utility
    maliput_integration::integration
)

add_executable(maliput_to_string
  maliput_to_string.cc
)

target_link_libraries(maliput_to_string
    gflags
    maliput::common
    maliput::utility
    maliput_integration::integration
)

add_executable(maliput_query
  maliput_query.cc
)

target_link_libraries(maliput_query
    gflags
    maliput::common
    maliput::base
    maliput::plugin
    maliput::routing
    maliput::utility
    maliput_malidrive::loader
    maliput_integration::integration
    maliput_object::api
    maliput_object::base
)

add_executable(maliput_derive_lane_s_routes
  maliput_derive_lane_s_routes.cc
)

target_link_libraries(maliput_derive_lane_s_routes
    gflags
    maliput::common
    maliput::routing
    maliput::utility
    maliput_integration::integration
)

add_executable(maliput_measure_load_time
  maliput_measure_load_time.cc
)

target_link_libraries(maliput_measure_load_time
    gflags
    maliput::common
    maliput::routing
    maliput::utility
    maliput_integration::integration
)

add_executable(maliput_to_string_with_plugin
  maliput_to_string_with_plugin.cc
)

target_link_libraries(maliput_to_string_with_plugin
  gflags
  maliput::api
  maliput::plugin
  maliput::common
  maliput::utility
)

add_executable(maliput_dynamic_environment
  maliput_dynamic_environment.cc
)

target_link_libraries(maliput_dynamic_environment
  gflags
  maliput::api
  maliput::plugin
  maliput::common
  maliput::utility
  maliput_integration::integration
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
