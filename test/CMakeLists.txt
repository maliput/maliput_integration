find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_pytest REQUIRED)

ament_add_pytest_test(maliput_to_obj_test maliput_to_obj_test.py WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin TIMEOUT 360)

if (TARGET maliput_to_obj_test)
add_dependencies(maliput_to_obj_test maliput_to_obj)
endif()

ament_add_gtest(tools_test tools_test.cc)
target_include_directories(tools_test
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(tools_test
    integration
    maliput::api
    maliput_dragway::maliput_dragway
    maliput_multilane::maliput_multilane
)
