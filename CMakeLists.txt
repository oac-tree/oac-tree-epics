cmake_minimum_required(VERSION 3.15)
project(sequencer-plugin-epics VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(COVERAGE "Generate unit test coverage information" OFF)
# option(SUP_EPICS_EXPORT_BUILDTREE "Export build tree in /home/user/.cmake registry" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/src/cmake)
include(configuration)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_subdirectory(src/main/c++)
add_subdirectory(src/test/c++)

# include(installation)
