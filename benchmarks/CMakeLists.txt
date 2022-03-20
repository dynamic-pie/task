project(benchmarks)

add_executable(benchmarks_run benchmark.cpp)

find_package(benchmark REQUIRED)
target_link_libraries(benchmarks_run benchmark::benchmark)