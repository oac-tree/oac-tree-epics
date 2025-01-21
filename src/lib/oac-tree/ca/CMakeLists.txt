add_library(oac-tree-ca SHARED)

set_target_properties(oac-tree-ca PROPERTIES
  EXPORT_NAME oac-tree-ca
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(oac-tree-ca
  PRIVATE
  channel_access_client_variable.cpp
  channel_access_helper.cpp
  channel_access_read_instruction.cpp
  channel_access_write_instruction.cpp
)

target_include_directories(oac-tree-ca PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>
)

target_link_libraries(oac-tree-ca PUBLIC oac-tree::oac-tree sup-epics::sup-epics)

install(TARGETS oac-tree-ca DESTINATION ${PLUGIN_PATH})
