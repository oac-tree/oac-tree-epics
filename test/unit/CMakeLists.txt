set(unit-tests oac-tree-epics-unit-tests)

add_executable(${unit-tests})

set_target_properties(${unit-tests} PROPERTIES OUTPUT_NAME "unit-tests")
set_target_properties(${unit-tests} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

target_sources(${unit-tests}
  PRIVATE
  channel_access_client_variable_tests.cpp
  channel_access_read_instruction_tests.cpp
  channel_access_write_instruction_tests.cpp
  global_ioc_environment.cpp
  test_user_interface.cpp
  pv_access_client_variable_tests.cpp
  pv_access_read_instruction_tests.cpp
  pv_access_server_variable_tests.cpp
  pv_access_write_instruction_tests.cpp
  rpc_client_instruction_tests.cpp
  unit_test_helper.cpp
)

target_include_directories(${unit-tests}
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

find_package(GTest REQUIRED)

target_link_libraries(${unit-tests} PUBLIC GTest::GTest GTest::Main oac-tree-ca oac-tree-pvxs sup-epics::sup-epics-test sup-protocol::sup-protocol)

gtest_discover_tests(${unit-tests} DISCOVERY_MODE PRE_TEST)
