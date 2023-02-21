add_executable(unit-tests)

target_sources(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_client_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_read_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_write_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/global_ioc_environment.cpp
        ${CMAKE_CURRENT_LIST_DIR}/log_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/test_user_interface.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_client_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_read_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_server_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_write_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/rpc_client_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_call_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_clock_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/unit_test_helper.cpp
)

target_include_directories(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

find_package(GTest REQUIRED)

target_link_libraries(unit-tests PUBLIC GTest::GTest GTest::Main sequencer-ca sequencer-misc sequencer-pvxs sup-dto::sup-rpc sup-epics::sup-epics-test)

set_target_properties(unit-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

gtest_discover_tests(unit-tests)
