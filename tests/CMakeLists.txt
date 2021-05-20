# gtest
find_package(GTest)
link_libraries(GTest::GTest GTest::Main)
include_directories(${GTEST_INCLUDE_DIRS})

link_libraries(pthread)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} test_source)
add_executable(vrb_test ${test_source})
target_link_libraries(vrb_test vrb)
