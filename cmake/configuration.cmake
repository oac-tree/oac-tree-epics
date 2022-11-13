
# -----------------------------------------------------------------------------
# Find if we are on CODAC infrastructure
# -----------------------------------------------------------------------------

if (DEFINED ENV{CODAC_ROOT})
  message(STATUS "CODAC environment detected at $ENV{CODAC_ROOT}")
  set(SUP_SEQUENCER_PLUGIN_EPICS_CODAC ON)
else()
  message(STATUS "No CODAC environment detected")
  set(SUP_SEQUENCER_PLUGIN_EPICS_CODAC OFF)
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

if (NOT SUP_SEQUENCER_PLUGIN_EPICS_CODAC)
  find_package(sup-dto REQUIRED)
endif()

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
if (COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage --coverage")
  message(INFO " Coverage enabled ${CMAKE_CXX_FLAGS}")
endif()
