add_library(sequencer-misc SHARED)

set_target_properties(sequencer-misc PROPERTIES
    VERSION ${LIBVERSION}
    SOVERSION ${LIBSOVERSION}
)

target_sources(sequencer-misc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/log_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_call_instruction.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/SystemClockVariable.cpp
)

target_include_directories(sequencer-misc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{CODAC_ROOT}/include
)

target_link_directories(sequencer-misc
    PUBLIC $ENV{CODAC_ROOT}/lib
)

target_link_libraries(sequencer-misc
    PUBLIC
        sequencer
        sup-dto
)

install(TARGETS sequencer-misc DESTINATION lib)
