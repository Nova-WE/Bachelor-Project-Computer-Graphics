cmake_minimum_required(VERSION 3.10)

project(BachelorProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main)

target_sources(main
    PRIVATE
        src/main.cc
)

find_package(OpenMP REQUIRED)
target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)

add_compile_options($<$<CONFIG:Release>:-O3>)
