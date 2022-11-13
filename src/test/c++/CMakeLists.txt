enable_testing()

find_package(GTest REQUIRED)
find_package(Threads)
include(GoogleTest)

add_subdirectory(unit)
