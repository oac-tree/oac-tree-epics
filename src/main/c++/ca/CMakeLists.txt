add_library(sequencer-ca SHARED)

set_target_properties(sequencer-ca PROPERTIES
    EXPORT_NAME sequencer-ca
    SOVERSION ${SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION}
    VERSION ${SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
)

target_sources(sequencer-ca
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_client_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_helper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_read_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/channel_access_write_instruction.cpp
)

target_include_directories(sequencer-ca PRIVATE ${CMAKE_CURRENT_LIST_DIR})

if(SUP_SEQUENCER_PLUGIN_EPICS_CODAC)
  target_include_directories(sequencer-ca PRIVATE $ENV{CODAC_ROOT}/include)
  target_link_directories(sequencer-ca PUBLIC $ENV{CODAC_ROOT}/lib)
  target_link_libraries(sequencer-ca PUBLIC sequencer sup-epics sup-dto)
else()
  target_link_libraries(sequencer-ca PUBLIC sequencer::sequencer sup-epics::sup-epics)
endif()

install(TARGETS sequencer-ca DESTINATION ${CMAKE_INSTALL_LIBDIR})
