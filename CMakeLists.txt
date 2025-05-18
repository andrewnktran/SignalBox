cmake_minimum_required(VERSION 3.10)
project(SignalBox)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(SignalBox
    src/main.cpp
    src/producer.cpp
    src/consumer.cpp
    src/parser.cpp
 "include/shared_types.h")

enable_testing()
add_subdirectory(tests)
