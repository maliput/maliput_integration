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

add_executable(yaml_to_obj
  yaml_to_obj.cc)

ament_target_dependencies(yaml_to_obj
  "maliput"
  "multilane"
)

target_link_libraries(yaml_to_obj
    gflags
    maliput::utilities
    multilane::multilane
    yaml-cpp
)

##############################################################################
# Install
##############################################################################

install(TARGETS dragway_to_urdf yaml_to_obj
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
