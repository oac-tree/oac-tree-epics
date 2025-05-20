add_library(oac-tree-pvxs SHARED)

set_target_properties(oac-tree-pvxs PROPERTIES
  EXPORT_NAME oac-tree-pvxs
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(oac-tree-pvxs
  PRIVATE
  pv_access_client_variable.cpp
  pv_access_helper.cpp
  pv_access_read_instruction.cpp
  pv_access_server_variable.cpp
  pv_access_shared_server_registry.cpp
  pv_access_shared_server.cpp
  pv_access_write_instruction.cpp
  rpc_client_instruction.cpp
)

target_include_directories(oac-tree-pvxs PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>
)

target_link_libraries(oac-tree-pvxs PUBLIC oac-tree::oac-tree sup-epics::sup-epics)

install(TARGETS oac-tree-pvxs DESTINATION ${PLUGIN_PATH})
