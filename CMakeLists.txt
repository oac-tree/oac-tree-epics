cmake_minimum_required(VERSION 3.15)
project(sup-sequencer-plugin-epics VERSION 2.0.0 LANGUAGES CXX)

option(COVERAGE "Generate unit test coverage information" OFF)
option(COA_BUILD_TESTS "Build unit tests" ON)
option(COA_BUILD_DOCUMENTATION "Build documentation" OFF)
option(COA_NO_CODAC "Don't look for the presence of CODAC environment" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/module)
set(CMAKE_CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/config)
include(configuration)

add_subdirectory(src)
if (COA_BUILD_TESTS)
  add_subdirectory(test)
endif()
add_subdirectory(doc)
