# -----------------------------------------------------------------------------
# Modules
# -----------------------------------------------------------------------------

include(CTest)
include(GNUInstallDirs)
include(FindPackageMessage)

# -----------------------------------------------------------------------------
# C++ version
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# CODAC enviorenment
# -----------------------------------------------------------------------------

if(NOT COA_NO_CODAC)
  find_package(CODAC OPTIONAL_COMPONENTS site-packages Python MODULE QUIET)
endif()

if(CODAC_FOUND)
  set(CODAC_FOUND_MESSAGE "Building with CODAC")

  # Append CODAC_CMAKE_PREFIXES to cmake seard directories, this helps cmake find packages installed in the CODAC enviorenment
  list(APPEND CMAKE_PREFIX_PATH ${CODAC_CMAKE_PREFIXES})

  # If CODAC module provides python executable, override Python3_EXECUTABLE with it
  if(CODAC_Python_FOUND AND NOT Python3_EXECUTABLE)
    set(Python3_EXECUTABLE ${CODAC_PYTHON_EXECUTABLE})
  endif()

  # Check if operating inside a CODAC CICD system
  if(CODAC_CICD)
    string(APPEND CODAC_FOUND_MESSAGE " CICD environment")

    set(COA_BUILD_DOCUMENTATION ON)
  else()
    string(APPEND CODAC_FOUND_MESSAGE " environment")
  endif()

  if(CODAC_DOCS)
    string(APPEND CODAC_FOUND_MESSAGE " and will build documentation")

    set(COA_BUILD_DOCUMENTATION ON)
  endif()

  find_package_message(
    CODAC_DETAILS
    "${CODAC_FOUND_MESSAGE}: ${CODAC_DIR} (version \"${CODAC_VERSION}\")"
    "[${CODAC_FOUND}][${CODAC_DIR}][${CODAC_CICD}][v${CODAC_VERSION}]"
  )
else()
  find_package_message(CODAC_DETAILS "Building without CODAC environment" "[${CODAC_FOUND}]")
endif()

# -----------------------------------------------------------------------------
# Variables
# -----------------------------------------------------------------------------

if(COVERAGE)
  # On coverage builds  alsways skip building docs and build tests
  set(COA_BUILD_DOCUMENTATION OFF)
  set(COA_BUILD_TESTS ON)
endif()

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
message(DEBUG "PLUGIN_PATH: ${PLUGIN_PATH}")

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
find_package(sup-protocol REQUIRED)
find_package(sequencer REQUIRED)
find_package(sup-epics REQUIRED)

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
if (COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage --coverage")
  message(STATUS "Enabling test coverage information")
endif()
