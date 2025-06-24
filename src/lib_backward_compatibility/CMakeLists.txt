# ca library compatibility
add_library(sequencer-ca SHARED)

set_target_properties(sequencer-ca PROPERTIES
  EXPORT_NAME sequencer-ca
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(sequencer-ca
  PRIVATE
  dummy.cpp
)

target_link_libraries(sequencer-ca PUBLIC oac-tree-ca)

install(TARGETS sequencer-ca DESTINATION ${PLUGIN_PATH})

# pvxs library compatibility
add_library(sequencer-pvxs SHARED)

set_target_properties(sequencer-pvxs PROPERTIES
  EXPORT_NAME sequencer-pvxs
  SOVERSION ${LIBSOVERSION}
  VERSION ${LIBVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(sequencer-pvxs
  PRIVATE
  dummy.cpp
)

target_link_libraries(sequencer-pvxs PUBLIC oac-tree-pvxs)

install(TARGETS sequencer-pvxs DESTINATION ${PLUGIN_PATH})
