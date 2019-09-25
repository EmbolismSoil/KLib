cmake_minimum_required (VERSION 3.5)

SET (INC_PATH . ${PROJECT_SOURCE_DIR}/KLib)
include_directories(${INC_PATH})
LINK_DIRECTORIES("/usr/lib/x86_64-linux-gnu/")

add_executable(test_lru test_lru.cpp)
target_link_libraries(test_lru boost_system)

add_executable(test_tostr test_tostr.cpp)
target_link_libraries(test_tostr boost_system)

add_executable(test_rate_limit test_rate_limit.cpp)
target_link_libraries(test_rate_limit boost_system boost_thread)

#add_executable(test_lda test_lda.cpp ../algorithm/LDA.pb.cc)
#target_link_libraries(test_lda protobuf)

#add_executable(test_load_lda test_load_lda.cpp ../algorithm/LDA.pb.cc)
#target_link_libraries(test_load_lda protobuf)

add_executable(test_timer_queue test_timer_queue.cpp)
target_link_libraries(test_timer_queue boost_system boost_thread boost_chrono)
