##############################################################################
# Sources
##############################################################################
add_executable(maliput_to_urdf
  maliput_to_urdf.cc
)

ament_target_dependencies(maliput_to_urdf
  "maliput"
  "maliput_dragway"
  "maliput_malidrive"
  "maliput_multilane"
)

target_link_libraries(maliput_to_urdf
    gflags
    integration
    maliput::common
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_malidrive::loader
    maliput_multilane::maliput_multilane
)

add_executable(maliput_to_obj
  maliput_to_obj.cc)

ament_target_dependencies(maliput_to_obj
  "maliput"
  "maliput_dragway"
  "maliput_malidrive"
  "maliput_multilane"
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
  maliput_to_string.cc)

ament_target_dependencies(maliput_to_string
  "maliput"
  "maliput_dragway"
  "maliput_malidrive"
  "maliput_multilane"
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
  maliput_query.cc)

ament_target_dependencies(maliput_query
  "maliput"
  "maliput_dragway"
  "maliput_malidrive"
  "maliput_multilane"
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
  maliput_derive_lane_s_routes.cc)

ament_target_dependencies(maliput_derive_lane_s_routes
  "maliput"
  "maliput_dragway"
  "maliput_malidrive"
  "maliput_multilane"
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

add_executable(maliput_to_string_with_plugin
  maliput_to_string_with_plugin.cc)

ament_target_dependencies(maliput_to_string_with_plugin
   "maliput"
)

target_link_libraries(maliput_to_string_with_plugin
  gflags
  maliput::api
  maliput::plugin
  maliput::common
  maliput::utilities
)

install(
  TARGETS maliput_to_string_with_plugin
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

##############################################################################
# Install
##############################################################################

install(
  TARGETS
    maliput_derive_lane_s_routes
    maliput_query
    maliput_to_obj
    maliput_to_string
    maliput_to_urdf
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

add_subdirectory(python)
