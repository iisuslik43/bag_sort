cmake_minimum_required(VERSION 2.8)
project(test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -lgtest_main -lgtest -lpthread")
include_directories(../)

set(SOURCE_FILES test.cpp ../headers.cpp ../writing.cpp ../config.cpp)
add_executable(test ${SOURCE_FILES})
