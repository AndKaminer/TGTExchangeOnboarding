cmake_minimum_required(VERSION 3.14)
project(Exchange CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(cpp/third_party/drogon)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(application
  cpp/src/main.cpp
  cpp/src/api_controller.cpp
  cpp/src/matching_engine.cpp
)
target_include_directories(application PRIVATE cpp/include)
target_link_libraries(application PRIVATE drogon)
