##############################################################################
# Sources
##############################################################################

add_library(integration tools.cc)

target_include_directories(integration
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

ament_target_dependencies(integration
  PUBLIC
    "maliput"
    "maliput_dragway"
    "maliput_malidrive"
    "maliput_multilane"
)

target_link_libraries(integration
  PUBLIC
    maliput::common
    maliput_malidrive::builder
    maliput_malidrive::loader
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
    RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

ament_export_libraries(integration)
