add_library(sequencer-pvxs SHARED)

set_target_properties(sequencer-pvxs PROPERTIES
    VERSION ${LIBVERSION}
    SOVERSION ${LIBSOVERSION}
    )

target_sources(sequencer-pvxs
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/RPCClientInstruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PVMonitorCache.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PVMonitorVariable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PVMonitorInstruction.cpp
    )

target_include_directories(sequencer-pvxs
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{CODAC_ROOT}/include
        $ENV{CODAC_ROOT}/include/sup
    )

target_link_directories(sequencer-pvxs
    PUBLIC $ENV{CODAC_ROOT}/lib
    )

target_link_libraries(sequencer-pvxs
    PUBLIC
        ccs-common
        ccs-types
        ccs-base
        ccs-pvxs
        sequencer
    )

target_compile_definitions(sequencer-pvxs
    PRIVATE
## No DEBUG level messages compiled in
#        LOG_DEBUG_ENABLE
    )

install(TARGETS sequencer-pvxs DESTINATION lib)
