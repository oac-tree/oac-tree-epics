# -----------------------------------------------------------------------------
# Modules
# -----------------------------------------------------------------------------

include(CTest)
include(GNUInstallDirs)

# -----------------------------------------------------------------------------
# CODAC enviorenment
# -----------------------------------------------------------------------------

if(NOT NO_CODAC)
  # cmake warns for the existance of ``<PackageName>_ROOT`` (CODAC_ROOT in this case) variables and ignores them
  # for compatibility reasons, we set the related policy to NEW behaviour to suppress warnings and enable desired behaviour
  cmake_policy(SET CMP0074 NEW)
  find_package(CODAC OPTIONAL_COMPONENTS site-packages Python MODULE)
endif()
if (CODAC_FOUND)
  # Append CODAC_CMAKE_PREFIXES to cmake seard directories, this helps cmake find packages installed in the CODAC enviorenment 
  list(APPEND CMAKE_PREFIX_PATH ${CODAC_CMAKE_PREFIXES})

  # If CODAC module provides python executable, override Python3_EXECUTABLE with it
  if(CODAC_Python_FOUND AND NOT Python3_EXECUTABLE)
    set(Python3_EXECUTABLE ${CODAC_PYTHON_EXECUTABLE})
  endif()
else()
  message(STATUS "Compiling without CODAC")
endif()

# -----------------------------------------------------------------------------
# Variables
# -----------------------------------------------------------------------------

set(SUP_SEQUENCER_PLUGIN_EPICS_SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set(SUP_SEQUENCER_PLUGIN_EPICS_BUILDVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# -----------------------------------------------------------------------------
# Directories
# -----------------------------------------------------------------------------

if (NOT DEFINED PLUGIN_PATH)
  set(PLUGIN_PATH lib/sequencer/plugins)
endif()
message(STATUS "PLUGIN_PATH: ${PLUGIN_PATH}")

if (NOT DEFINED PLUGIN_RUNTIME_DIRECTORY)
  set(PLUGIN_RUNTIME_DIRECTORY ${CMAKE_BINARY_DIR}/${PLUGIN_PATH})
endif()

if (NOT DEFINED TEST_OUTPUT_DIRECTORY)
  set(TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)
endif()

file(MAKE_DIRECTORY ${PLUGIN_RUNTIME_DIRECTORY})
file(MAKE_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

find_package(sup-dto REQUIRED)
find_package(sequencer REQUIRED)
find_package(sup-epics REQUIRED)

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
if (COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage --coverage")
  message(INFO " Coverage enabled ${CMAKE_CXX_FLAGS}")
endif()
