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
  cpp/src/market_data_controller.cpp
  cpp/src/matching_engine.cpp
)
target_include_directories(application PRIVATE cpp/include)
target_link_libraries(application PRIVATE drogon)

option(EXCHANGE_BUILD_TESTS "Build the matching engine tests" ON)
if(EXCHANGE_BUILD_TESTS)
  enable_testing()
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  add_subdirectory(cpp/third_party/googletest)

  add_executable(matching_engine_tests
    cpp/tests/order_queue_test.cpp
    cpp/tests/matching_engine_test.cpp
    cpp/src/matching_engine.cpp
  )
  target_include_directories(matching_engine_tests PRIVATE cpp/include)
  target_link_libraries(matching_engine_tests PRIVATE GTest::gtest_main)

  include(GoogleTest)
  # Timeout so a matching loop that never terminates fails instead of hanging
  gtest_discover_tests(matching_engine_tests PROPERTIES TIMEOUT 5)
endif()
