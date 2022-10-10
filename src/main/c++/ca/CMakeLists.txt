add_library(sequencer-ca SHARED)

set_target_properties(sequencer-ca PROPERTIES
    VERSION ${LIBVERSION}
    SOVERSION ${LIBSOVERSION}
)

target_sources(sequencer-ca
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_client_variable.cpp
)

target_include_directories(sequencer-ca
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{CODAC_ROOT}/include
)

target_link_directories(sequencer-ca
    PUBLIC $ENV{CODAC_ROOT}/lib
)

target_link_libraries(sequencer-ca
    PUBLIC
        sequencer
        sup-epics
        sup-dto
)

install(TARGETS sequencer-ca DESTINATION lib)
