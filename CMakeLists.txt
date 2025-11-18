cmake_minimum_required(VERSION 4.0)

project(ugp CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_LINKER_TYPE MOLD)

find_package(Vulkan REQUIRED)

add_compile_options(-ftime-trace)

set(ALLOW_EXTERNAL_SPIRV_TOOLS TRUE)

add_subdirectory(ext/glslang EXCLUDE_FROM_ALL)

include_directories(include ext/glslang)

add_executable(prototype samples/prototype.cpp)
add_executable(traditional samples/traditional.cpp)

target_precompile_headers(prototype PRIVATE <vulkan/vulkan.hpp>)
target_precompile_headers(traditional REUSE_FROM prototype)

target_link_libraries(prototype
	fmt
	glfw
	Vulkan::Vulkan
	glslang::glslang
	glslang::glslang-default-resource-limits
)

target_link_libraries(traditional fmt glfw Vulkan::Vulkan)
