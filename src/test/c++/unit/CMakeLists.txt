add_executable(unit-tests)

target_sources(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/null_user_interface.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessClient-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessInstruction-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessVariable-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/RPCClientInstruction-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVAccessWriteInstruction-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVMonitorInstruction-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVMonitorVariable-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVServerVariable-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVClientVariable-tests.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/SystemCallInstruction-tests.cpp
)

target_include_directories(unit-tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../../main/c++/include
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
        sequencer
        # sequencer-ca
        sequencer-misc
        # sequencer-pvxs
        sup-dto
)
