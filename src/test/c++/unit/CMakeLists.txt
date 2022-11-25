add_executable(unit-tests)

target_sources(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_client_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/global_ioc_environment.cpp
        ${CMAKE_CURRENT_LIST_DIR}/log_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/null_user_interface.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_client_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_server_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_write_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/softioc_runner.cpp
        ${CMAKE_CURRENT_LIST_DIR}/softioc_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_call_instruction_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_clock_variable_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/unit_test_helper.cpp
)

target_include_directories(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../../main/c++
        $ENV{CODAC_ROOT}/include
)

target_link_directories(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../../../target/lib
        $ENV{CODAC_ROOT}/lib
)

find_package(GTest REQUIRED)

target_link_libraries(unit-tests
    PUBLIC
        GTest::GTest
        GTest::Main
        sequencer-ca
        sequencer-misc
        sequencer-pvxs
)
