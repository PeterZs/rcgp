cmake_minimum_required(VERSION 4.0)

project(ugp CXX)

set(CMAKE_CXX_STANDARD 26)

find_package(Vulkan REQUIRED)

add_executable(main main.cpp)

target_link_libraries(main fmt glfw Vulkan::Vulkan)
