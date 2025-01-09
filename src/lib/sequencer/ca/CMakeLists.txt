add_library(sequencer-ca SHARED)

set_target_properties(sequencer-ca PROPERTIES
  EXPORT_NAME sequencer-ca
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
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
