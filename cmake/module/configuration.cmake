# -----------------------------------------------------------------------------
# Common project configuration
# -----------------------------------------------------------------------------

include(COACommonConfig)
include(COASetupCodacEnvironment)

# -----------------------------------------------------------------------------
# Directories
# -----------------------------------------------------------------------------
if (NOT DEFINED PLUGIN_PATH)
  set(PLUGIN_PATH ${CMAKE_INSTALL_LIBDIR}/oac-tree/plugins)
endif()
message(DEBUG "PLUGIN_PATH: ${PLUGIN_PATH}")

if (NOT DEFINED PLUGIN_RUNTIME_DIRECTORY)
  set(PLUGIN_RUNTIME_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_PATH})
endif()

file(MAKE_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY})

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(sup-dto REQUIRED)
find_package(sup-protocol REQUIRED)
find_package(oac-tree REQUIRED)
find_package(sup-epics REQUIRED)
