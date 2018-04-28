cmake_minimum_required (VERSION 3.5)

SET (INC_PATH . ${PROJECT_SOURCE_DIR}/KLib)
include_directories(${INC_PATH})
add_executable(test_lru test_lru.cpp)
target_link_libraries(test_lru boost_system)
add_executable(test_tostr test_tostr.cpp)
target_link_libraries(test_tostr boost_system)
