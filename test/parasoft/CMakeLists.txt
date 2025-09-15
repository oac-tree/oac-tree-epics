if(NOT COA_PARASOFT_INTEGRATION)
  return()
endif()

include(COASetupParasoft)

# Add parasoft project for oac-tree-plugin-epics
# This generates the build time information needed by Parasoft for analysis
cpptest_add_executable(oac-tree-plugin-epics-cpptest
  CPPTEST_PROJECT_LOC ${CMAKE_CURRENT_BINARY_DIR}
  CPPTEST_PROJECT_FOLDERS
    oac-tree-ca=${CMAKE_SOURCE_DIR}/src/lib/oac-tree/ca
    oac-tree-pvxs=${CMAKE_SOURCE_DIR}/src/lib/oac-tree/pvxs
  TARGETS oac-tree-ca oac-tree-pvxs
)

get_target_property(OAC_TREE_CA_INCLUDE_DIRECTORIES oac-tree-ca INCLUDE_DIRECTORIES)
get_target_property(OAC_TREE_PVXS_INCLUDE_DIRECTORIES oac-tree-pvxs INCLUDE_DIRECTORIES)

set(OAC_TREE_PLUGIN_EPICS_INCLUDE_DIRECTORIES
    ${OAC_TREE_CA_INCLUDE_DIRECTORIES}
    ${OAC_TREE_PVXS_INCLUDE_DIRECTORIES}
)

list(REMOVE_DUPLICATES OAC_TREE_PLUGIN_EPICS_INCLUDE_DIRECTORIES)

target_include_directories(oac-tree-plugin-epics-cpptest PUBLIC ${OAC_TREE_PLUGIN_EPICS_INCLUDE_DIRECTORIES})

target_link_libraries(oac-tree-plugin-epics-cpptest
  PUBLIC
  sup-epics::sup-epics
  oac-tree::oac-tree
  oac-tree-ca
  oac-tree-pvxs
)
