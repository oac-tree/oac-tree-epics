add_executable(unit-tests)

target_sources(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/LogVariable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SystemCallInstruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/NullUserInterface.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessVariable-tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PVMonitorVariable-tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SystemCallInstruction-tests.cpp
    )

target_include_directories(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../../main/c++/ca
        ${CMAKE_CURRENT_LIST_DIR}/../../../main/c++/pvxs
        $ENV{CODAC_ROOT}/include
        $ENV{CODAC_ROOT}/include/sup
    )

target_link_directories(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../../../target/lib
        $ENV{CODAC_ROOT}/lib
    )

find_package(GTest REQUIRED)

target_link_libraries(unit-tests sequencer sequencer-ca sequencer-pvxs GTest::GTest GTest::Main ccs-types ccs-base ccs-common ccs-ca ccs-pvxs)
