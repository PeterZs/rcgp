cmake_minimum_required(VERSION 4.0)

project(ugp CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_LINKER_TYPE MOLD)

find_package(Vulkan REQUIRED)

add_compile_options(-ftime-trace)

add_executable(main main.cpp)

target_precompile_headers(main PRIVATE <vulkan/vulkan.hpp>)

target_link_libraries(main fmt glfw Vulkan::Vulkan)
