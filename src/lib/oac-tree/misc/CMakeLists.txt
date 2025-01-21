add_library(oac-tree-misc SHARED)

set_target_properties(oac-tree-misc PROPERTIES
  EXPORT_NAME oac-tree-misc
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(oac-tree-misc
  PRIVATE
  system_call_instruction.cpp
  system_clock_variable.cpp
)

target_include_directories(oac-tree-misc PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>
)

target_link_libraries(oac-tree-misc PUBLIC oac-tree::oac-tree)

install(TARGETS oac-tree-misc DESTINATION ${PLUGIN_PATH})
