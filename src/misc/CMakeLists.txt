add_library(sequencer-misc SHARED)

set_target_properties(sequencer-misc PROPERTIES
    EXPORT_NAME sequencer-misc
    SOVERSION ${SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION}
    VERSION ${SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
)

target_sources(sequencer-misc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/log_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_call_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_clock_variable.cpp
)

target_include_directories(sequencer-misc PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(sequencer-misc PUBLIC sequencer::sequencer)

install(TARGETS sequencer-misc DESTINATION ${CMAKE_INSTALL_LIBDIR})
