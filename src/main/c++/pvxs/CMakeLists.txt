add_library(sequencer-pvxs SHARED)

set_target_properties(sequencer-pvxs PROPERTIES
    EXPORT_NAME sequencer-pvxs
    SOVERSION ${SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION}
    VERSION ${SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION}
)

target_sources(sequencer-pvxs
    PRIVATE
        # ${CMAKE_CURRENT_LIST_DIR}/RPCClientInstruction.cpp
        # ${CMAKE_CURRENT_LIST_DIR}/PVServerVariable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_client_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_helper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_server_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pv_access_write_instruction.cpp
)

target_include_directories(sequencer-pvxs PRIVATE ${CMAKE_CURRENT_LIST_DIR})

if(SUP_SEQUENCER_PLUGIN_EPICS_CODAC)
  target_include_directories(sequencer-pvxs PRIVATE $ENV{CODAC_ROOT}/include)
  target_link_directories(sequencer-pvxs PUBLIC $ENV{CODAC_ROOT}/lib)
  target_link_libraries(sequencer-pvxs PUBLIC sequencer sup-epics sup-dto)
else()
  target_link_libraries(sequencer-pvxs PUBLIC sequencer::sequencer sup-epics::sup-epics)
endif()

install(TARGETS sequencer-pvxs DESTINATION ${CMAKE_INSTALL_LIBDIR})
