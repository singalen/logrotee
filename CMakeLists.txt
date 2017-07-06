cmake_minimum_required(VERSION 3.8)
project(logrotee)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES logrotee.cpp)
add_executable(logrotee ${SOURCE_FILES})