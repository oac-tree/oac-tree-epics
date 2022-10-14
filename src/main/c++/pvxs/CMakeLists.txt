add_library(sequencer-pvxs SHARED)

set_target_properties(sequencer-pvxs PROPERTIES
    VERSION ${LIBVERSION}
    SOVERSION ${LIBSOVERSION}
)

target_sources(sequencer-pvxs
    PRIVATE
        # ${CMAKE_CURRENT_LIST_DIR}/RPCClientInstruction.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVAccessWriteInstruction.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVMonitorCache.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVMonitorVariable.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVMonitorInstruction.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVServerVariable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_client_variable.cpp
)

target_include_directories(sequencer-pvxs
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{CODAC_ROOT}/include
)

target_link_directories(sequencer-pvxs
    PUBLIC $ENV{CODAC_ROOT}/lib
)

target_link_libraries(sequencer-pvxs
    PUBLIC
        sequencer
        sup-epics
        sup-dto
)

install(TARGETS sequencer-pvxs DESTINATION lib)
