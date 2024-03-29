##############################################################################
# Project
##############################################################################

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(maliput_integration LANGUAGES C CXX VERSION 3.0.0)

##############################################################################
# Find 3rd Party Packages
##############################################################################

message(STATUS "\n\n====== Finding 3rd Party Packages ======\n")

find_package(ament_cmake REQUIRED)
find_package(gflags REQUIRED)
find_package(maliput REQUIRED)
find_package(maliput_dragway REQUIRED)
find_package(maliput_malidrive REQUIRED)
find_package(maliput_multilane REQUIRED)
find_package(maliput_object REQUIRED)
find_package(maliput_sparse REQUIRED)
find_package(maliput_osm REQUIRED)
find_package(maliput_py REQUIRED)
find_package(yaml-cpp REQUIRED)

##############################################################################
# Project Configuration
##############################################################################

message(STATUS "\n\n========= Project Configuration ========\n")

set(BUILD_SHARED_LIBS true)

include(${PROJECT_SOURCE_DIR}/cmake/DefaultCFlags.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/SanitizersConfig.cmake)

##############################################################################
# Sources
##############################################################################

add_subdirectory(src)

##############################################################################
# Tests
##############################################################################

# We always enable tests (that might change in the future)
if(BUILD_TESTING)
  find_package(ament_cmake_clang_format REQUIRED)
  find_package(ament_cmake_flake8 REQUIRED)
  enable_testing()
  add_subdirectory(test)
  ament_flake8("--config" ${CMAKE_CURRENT_SOURCE_DIR}/.flake8)
  ament_clang_format(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format)
endif()

##############################################################################
# Docs
##############################################################################

if(BUILD_DOCS)
  message(STATUS "Doxygen generation - Enabled")
  find_package(ament_cmake_doxygen REQUIRED)
  ament_doxygen_generate(doxygen_maliput_integration
    CONFIG_OVERLAY doc/Doxyfile.overlay.in
    DEPENDENCIES maliput maliput_py maliput_malidrive maliput_dragway maliput_multilane maliput_object
    TEST_ON_WARNS
  )
  add_definitions(-DBUILD_DOCS)
else()
  message(STATUS "Doxygen generation - Disabled")
endif()

##############################################################################
# Export
##############################################################################

ament_export_dependencies(ament_cmake)
ament_package()
