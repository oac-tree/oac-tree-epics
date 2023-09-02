add_library(sequencer-misc SHARED)

target_compile_features(sequencer-misc PRIVATE cxx_std_11)

set_target_properties(sequencer-misc PROPERTIES
  EXPORT_NAME sequencer-misc
  SOVERSION ${SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION}
  VERSION ${SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
  LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(sequencer-misc
  PRIVATE
  system_call_instruction.cpp
  system_clock_variable.cpp
)

target_include_directories(sequencer-misc PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>
)

target_link_libraries(sequencer-misc PUBLIC sequencer::sequencer)

install(TARGETS sequencer-misc DESTINATION ${PLUGIN_PATH})
