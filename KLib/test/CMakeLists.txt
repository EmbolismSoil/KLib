cmake_minimum_required (VERSION 3.8)

SET (INC_PATH . ${PROJECT_SOURCE_DIR})
include_directories(${INC_PATH})
add_executable(test_lru test_lru.cpp)