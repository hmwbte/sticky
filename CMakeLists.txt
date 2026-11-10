cmake_minimum_required(VERSION 3.10)
project(STICKY)
set(CMAKE_CXX_STANDARD 20)

option(HMW_STICKY_BUILD_TESTS "Build the test program" ON)

add_library (sticky STATIC src/sticky.cpp)

if (HMW_STICKY_BUILD_TESTS)
  add_subdirectory(test)
endif()