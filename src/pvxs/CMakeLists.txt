add_library(sequencer-pvxs SHARED)

set_target_properties(sequencer-pvxs PROPERTIES
    EXPORT_NAME sequencer-pvxs
    SOVERSION ${SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION}
    VERSION ${SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY}
)

target_sources(sequencer-pvxs
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_client_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_helper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_read_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_server_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_write_instruction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/rpc_client_instruction.cpp
)

target_include_directories(sequencer-pvxs PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(sequencer-pvxs PUBLIC sequencer::sequencer sup-epics::sup-epics)

install(TARGETS sequencer-pvxs DESTINATION ${PLUGIN_PATH})
