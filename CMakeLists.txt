cmake_minimum_required(VERSION 3.10)
project(SDA_Benchmark)

set(CMAKE_CXX_STANDARD 17)
# Optimization flags are critical for fair benchmarking
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall")

set(SOURCES
    src/main.cpp
    src/utils.cpp
)

add_executable(sda_bench ${SOURCES})