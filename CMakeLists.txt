cmake_minimum_required(VERSION 3.10)

project(BachelorProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main)

target_sources(main
    PRIVATE
        src/main.cc
)

