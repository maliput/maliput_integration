find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_pytest REQUIRED)


# TODO(https://github.com/maliput/maliput_infrastructure/issues/222): Use ament_index for finding resources provided by the packages.
set(MALIPUT_MULTILANE_RESOURCE_PATH "${maliput_multilane_DIR}/..")
set(MALIPUT_MALIDRIVE_RESOURCE_PATH "${maliput_malidrive_DIR}/..")

ament_add_pytest_test(maliput_to_obj_test maliput_to_obj_test.py
  WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin TIMEOUT 360
  ENV "MULTILANE_RESOURCE_ROOT=${MALIPUT_MULTILANE_RESOURCE_PATH}"
)

if (TARGET maliput_to_obj_test)
add_dependencies(maliput_to_obj_test maliput_to_obj)
endif()


# tools_test
ament_add_gtest(tools_test tools_test.cc)
target_link_libraries(tools_test
    integration
    maliput::api
    maliput_dragway::maliput_dragway
    maliput_multilane::maliput_multilane
)

# timer_test
ament_add_gtest(timer_test timer_test.cc)
target_link_libraries(timer_test
    integration
)

# chrono_timer_test
ament_add_gtest(chrono_timer_test chrono_timer_test.cc)
target_link_libraries(chrono_timer_test
    integration
)

# create_timer_test
ament_add_gtest(create_timer_test create_timer_test.cc)
target_link_libraries(create_timer_test
    integration
)

# dynamic_environment_handler_test
ament_add_gtest(dynamic_environment_handler_test dynamic_environment_handler_test.cc)
target_link_libraries(dynamic_environment_handler_test
    integration
    maliput::test_utilities
)

# create_dynamic_environment_handler_test
ament_add_gtest(create_dynamic_environment_handler_test create_dynamic_environment_handler_test.cc)
target_link_libraries(create_dynamic_environment_handler_test
    integration
    maliput::test_utilities
)

# fixed_phase_iteration_handler_test
ament_add_gtest(fixed_phase_iteration_handler_test fixed_phase_iteration_handler_test.cc)
target_link_libraries(fixed_phase_iteration_handler_test
    integration
    maliput::api
)

target_compile_definitions(fixed_phase_iteration_handler_test
  PRIVATE
    DEF_MALIDRIVE_RESOURCES="${MALIPUT_MALIDRIVE_RESOURCE_PATH}"
)
