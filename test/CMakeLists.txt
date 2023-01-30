enable_testing()

find_package(GTest REQUIRED)
find_package(Threads)
include(GoogleTest)

add_subdirectory(unit)

file(WRITE ${TEST_OUTPUT_DIRECTORY}/test.sh
"#!/bin/bash
export TEST_RESOURCES_PATH=" ${CMAKE_CURRENT_SOURCE_DIR} "/resources
" ${TEST_OUTPUT_DIRECTORY} "/unit-tests \"$@\"
exit $?
")

configure_file(${TEST_OUTPUT_DIRECTORY}/test.sh ${TEST_OUTPUT_DIRECTORY}/test.sh 
               FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
