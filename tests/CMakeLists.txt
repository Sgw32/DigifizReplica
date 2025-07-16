cmake_minimum_required(VERSION 3.10)
project(rpm_filter_tests C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

add_library(rpm_filter STATIC ../ESP32/Digifiz/main/rpm_filter.c)

add_executable(rpm_filter_tests rpm_filter_test.cpp)

target_include_directories(rpm_filter_tests PRIVATE .. ../ESP32/Digifiz/main ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(rpm_filter_tests rpm_filter)
