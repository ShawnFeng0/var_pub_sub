cmake_minimum_required(VERSION 3.19)
project(var_pub_sub)

set(CMAKE_CXX_STANDARD 14)

# gtest
find_package(GTest)
link_libraries(GTest::GTest GTest::Main)
include_directories(${GTEST_INCLUDE_DIRS})

link_libraries(pthread)

add_executable(var_pub_sub tests/pub_sub_test.cc)
