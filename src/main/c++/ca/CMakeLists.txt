add_library(sequencer-ca SHARED)

set_target_properties(sequencer-ca PROPERTIES
    VERSION ${LIBVERSION}
    SOVERSION ${LIBSOVERSION}
    )

target_sources(sequencer-ca
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessClientContext.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessClient.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ChannelAccessVariable.cpp
    )

target_include_directories(sequencer-ca
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{CODAC_ROOT}/include
        $ENV{CODAC_ROOT}/include/sup
## ChannelAccess support now through static inline wrappers requires EPICS header files
        $ENV{CODAC_ROOT}/epics/include
        $ENV{CODAC_ROOT}/epics/include/os/Linux
        $ENV{CODAC_ROOT}/epics/include/compiler/gcc
    )

target_link_directories(sequencer-ca
    PUBLIC $ENV{CODAC_ROOT}/lib
    )

target_link_libraries(sequencer-ca
    PUBLIC
        ccs-common
        ccs-types
        ccs-base
        ccs-ca
        sequencer
    )

target_compile_definitions(sequencer-ca
    PRIVATE
        LOG_DEBUG_ENABLE
    )

install(TARGETS sequencer-ca DESTINATION lib)
