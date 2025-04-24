# -----------------------------------------------------------------------------
# Main project configuration
# -----------------------------------------------------------------------------

include(GNUInstallDirs)
include(CTest)
include(COASetupCodacEnvironment)

# -----------------------------------------------------------------------------
# C++ version
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# -----------------------------------------------------------------------------
# Variables
# -----------------------------------------------------------------------------
set(LIBVERSION ${CMAKE_PROJECT_VERSION})
set(LIBSOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR})

# -----------------------------------------------------------------------------
# Directories
# -----------------------------------------------------------------------------
if (NOT DEFINED PLUGIN_PATH)
  set(PLUGIN_PATH lib/oac-tree/plugins)
endif()
message(DEBUG "PLUGIN_PATH: ${PLUGIN_PATH}")

if (NOT DEFINED PLUGIN_RUNTIME_DIRECTORY)
  set(PLUGIN_RUNTIME_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_PATH})
endif()

if (NOT DEFINED TEST_OUTPUT_DIRECTORY)
  set(TEST_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_bin)
endif()

file(MAKE_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY})
file(MAKE_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(sup-dto REQUIRED)
find_package(sup-protocol REQUIRED)
find_package(oac-tree REQUIRED)
find_package(sup-epics REQUIRED)
