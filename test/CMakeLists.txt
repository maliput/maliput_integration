find_package(ament_cmake_pytest REQUIRED)

ament_add_pytest_test(maliput_to_obj_test maliput_to_obj_test.py WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin TIMEOUT 360)

if (TARGET maliput_to_obj_test)
  add_dependencies(maliput_to_obj_test maliput_to_obj)
endif()
