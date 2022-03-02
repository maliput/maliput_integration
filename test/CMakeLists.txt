find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_pytest REQUIRED)

ament_add_pytest_test(maliput_to_obj_test maliput_to_obj_test.py WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin TIMEOUT 360)

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
