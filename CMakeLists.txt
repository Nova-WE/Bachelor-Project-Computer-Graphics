cmake_minimum_required(VERSION 3.10)

project(RayTracing_in_a_Weekend LANGUAGES CXX)

find_package(Eigen3 REQUIRED NO_MODULE)

add_executable(main src/main.cc)
target_link_libraries(main PRIVATE Eigen3::Eigen)