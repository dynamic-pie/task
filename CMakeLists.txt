cmake_minimum_required(VERSION 3.21)
project(task)

set(CMAKE_CXX_STANDARD 17)

add_executable(task main.cpp lib/order_book.h lib/parser.h lib/base.h)

find_package(nlohmann_json 3.10.5 REQUIRED)

target_link_libraries(task PRIVATE nlohmann_json::nlohmann_json)
add_subdirectory(benchmarks)