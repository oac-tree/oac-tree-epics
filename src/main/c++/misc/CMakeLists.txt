add_library(sequencer-misc SHARED)

set_target_properties(sequencer-misc PROPERTIES
    EXPORT_NAME sequencer-misc
    SOVERSION ${SEQUENCER_PLUGIN_EPICS_SOVERSION}
    VERSION ${SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
)

target_sources(sequencer-misc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/log_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_call_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system_clock_variable.cpp
)

target_include_directories(sequencer-misc PRIVATE ${CMAKE_CURRENT_LIST_DIR})

if(SUP_SEQUENCER_PLUGIN_EPICS_CODAC)
  target_include_directories(sequencer-misc PRIVATE $ENV{CODAC_ROOT}/include)
  target_link_directories(sequencer-misc PUBLIC $ENV{CODAC_ROOT}/lib)
  target_link_libraries(sequencer-misc PUBLIC sequencer sup-epics sup-dto)
else()
  target_link_libraries(sequencer-misc PUBLIC sequencer::sequencer)
endif()

install(TARGETS sequencer-misc DESTINATION ${CMAKE_INSTALL_LIBDIR})
