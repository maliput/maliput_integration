##############################################################################
# Sources
##############################################################################
add_executable(dragway_to_urdf
  dragway_to_urdf.cc
)

ament_target_dependencies(dragway_to_urdf
  "maliput"
  "maliput_dragway"
)

target_link_libraries(dragway_to_urdf
    gflags
    maliput::common
    maliput::geometry_base
    maliput::utilities
    maliput_dragway::maliput_dragway
)

add_executable(maliput_to_obj
  maliput_to_obj.cc)

ament_target_dependencies(maliput_to_obj
  "maliput"
  "maliput_dragway"
  "maliput_multilane"
)

target_link_libraries(maliput_to_obj
    gflags
    maliput::utilities
    maliput_dragway::maliput_dragway
    maliput_multilane::maliput_multilane
    yaml-cpp
)

##############################################################################
# Install
##############################################################################

install(TARGETS dragway_to_urdf maliput_to_obj
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
