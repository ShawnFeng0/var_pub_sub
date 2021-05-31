cmake_minimum_required(VERSION 3.5)
project(var_pub_sub)

set(CMAKE_CXX_STANDARD 14)

option(VRB_BUILD_TESTS "Build tests" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(vrb src/vrb.cc)

# install vrb
install(TARGETS vrb
        ARCHIVE DESTINATION lib)

file(GLOB VRB_HEADERS include/vrb/*)
install(FILES ${VRB_HEADERS} DESTINATION include/vrb)

if (VRB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
