# -----------------------------------------------------------------------------
# Modules
# -----------------------------------------------------------------------------

include(CTest)
include(GNUInstallDirs)

# -----------------------------------------------------------------------------
# Find if we are on CODAC infrastructure
# -----------------------------------------------------------------------------

get_filename_component(SUP_SEQUENCER_PLUGIN_EPICS_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

if (DEFINED ENV{CODAC_ROOT})
  message(STATUS "CODAC environment detected at $ENV{CODAC_ROOT}")
  set(SUP_SEQUENCER_PLUGIN_EPICS_CODAC ON)
else()
  message(STATUS "No CODAC environment detected")
  set(SUP_SEQUENCER_PLUGIN_EPICS_CODAC OFF)
endif()

# -----------------------------------------------------------------------------
# Variables
# -----------------------------------------------------------------------------

set(SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set(SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# -----------------------------------------------------------------------------
# Directories
# -----------------------------------------------------------------------------

if (DEFINED ENV{CODAC_ROOT})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SUP_SEQUENCER_PLUGIN_EPICS_PROJECT_DIR}/target/bin)
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

if (NOT SUP_SEQUENCER_PLUGIN_EPICS_CODAC)
  find_package(sup-dto REQUIRED)
  find_package(sequencer REQUIRED)
  find_package(sup-epics REQUIRED)
endif()

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
if (COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage --coverage")
  message(INFO " Coverage enabled ${CMAKE_CXX_FLAGS}")
endif()
