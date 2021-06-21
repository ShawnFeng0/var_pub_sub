cmake_minimum_required(VERSION 3.5)
project(vrb)

set(CMAKE_CXX_STANDARD 14)

option(VRB_BUILD_TESTS "Build tests" OFF)

add_library(${PROJECT_NAME} src/vrb.cc)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# install vrb
install(TARGETS vrb
        ARCHIVE DESTINATION lib)

file(GLOB VRB_HEADERS include/vrb/*)
install(FILES ${VRB_HEADERS} DESTINATION include/vrb)

if (VRB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
