add_library(sequencer-pvxs SHARED)

set_target_properties(sequencer-pvxs PROPERTIES
  EXPORT_NAME sequencer-pvxs
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(sequencer-pvxs
  PRIVATE
  pv_access_client_variable.cpp
  pv_access_helper.cpp
  pv_access_read_instruction.cpp
  pv_access_server_variable.cpp
  pv_access_shared_server.cpp
  pv_access_write_instruction.cpp
  rpc_client_instruction.cpp
)

target_include_directories(sequencer-pvxs PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>
)

target_link_libraries(sequencer-pvxs PUBLIC sequencer::sequencer sup-epics::sup-epics)

install(TARGETS sequencer-pvxs DESTINATION ${PLUGIN_PATH})
