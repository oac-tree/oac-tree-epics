add_library(sequencer-misc SHARED)

set_target_properties(sequencer-misc PROPERTIES
    VERSION ${LIBVERSION}
    SOVERSION ${LIBSOVERSION}
    )

target_sources(sequencer-misc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/CopyInstruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LogInstruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RegisterTypeInstruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SystemCallInstruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SystemClockVariable.cpp
    )

target_include_directories(sequencer-misc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../include
        $ENV{CODAC_ROOT}/include
        $ENV{CODAC_ROOT}/include/sup
    )

target_link_directories(sequencer-misc
    PUBLIC $ENV{CODAC_ROOT}/lib
    )

target_link_libraries(sequencer-misc
    PUBLIC
        ccs-common
        ccs-types
        ccs-base
        sequencer
    )

target_compile_definitions(sequencer-misc
    PRIVATE
## No DEBUG level messages compiled in
#        LOG_DEBUG_ENABLE
    )

install(TARGETS sequencer-misc DESTINATION lib)
