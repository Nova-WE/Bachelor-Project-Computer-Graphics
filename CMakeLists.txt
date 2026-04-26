cmake_minimum_required(VERSION 3.10)

project(BachelorProject LANGUAGES CXX)

add_executable(main)

target_sources(main
    PRIVATE
        src/main.cc
)

