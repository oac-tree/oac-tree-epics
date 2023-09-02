add_library(sequencer-ca SHARED)

target_compile_features(sequencer-ca PRIVATE cxx_std_11)

set_target_properties(sequencer-ca PROPERTIES
  EXPORT_NAME sequencer-ca
  SOVERSION ${SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION}
  VERSION ${SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(sequencer-ca
  PRIVATE
  channel_access_client_variable.cpp
  channel_access_helper.cpp
  channel_access_read_instruction.cpp
  channel_access_write_instruction.cpp
)

target_include_directories(sequencer-ca PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>
)

target_link_libraries(sequencer-ca PUBLIC sequencer::sequencer sup-epics::sup-epics)

install(TARGETS sequencer-ca DESTINATION ${PLUGIN_PATH})
