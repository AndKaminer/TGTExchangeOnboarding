cmake_minimum_required(VERSION 3.14)
project(Exchange CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(main cpp/main.cpp)
target_include_directories(main PRIVATE cpp/include)
