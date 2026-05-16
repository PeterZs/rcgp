# RCGP

Reference implementation for *RCGP: Resource Contracts for Graphics
Programming* (SIGGRAPH 2026 / TOG).

<p align="center">
  <img src="docs/teaser.png" width="100%" />
</p>

RCGP is a header-and-static-library C++ project. This README explains how to
pull it into your own application.

## Requirements

- Clang 19+ or GCC 14+ (C++26)
- CMake ≥ 4.0
- Vulkan SDK (with ray tracing extensions enabled)
- `glslang`, `fmt`, `glfw`
- Python 3 (used by the build for source generation)

## Adding RCGP to a CMake project

Place the RCGP source tree somewhere under your project (for example as a
git submodule at `third_party/rcgp`) and pull it in with `add_subdirectory`:

```cmake
cmake_minimum_required(VERSION 4.0)
project(my_app CXX)

set(CMAKE_CXX_STANDARD 26)

find_package(Vulkan REQUIRED)
find_package(glslang REQUIRED)

add_subdirectory(third_party/rcgp EXCLUDE_FROM_ALL)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE rcgp)
```

That is the full integration. Linking against the `rcgp` target gives you:

- the `rcgp` static library,
- its public headers (the `include/` directory is added automatically),
- and all of its transitive dependencies — `fmt`, `glfw`, `Vulkan::Vulkan`,
  and the `glslang` libraries — so you do not have to repeat them on your
  own targets.

In `main.cpp`, include the umbrella header:

```cpp
#include <rcgp.hpp>
```

## Optional: glm interop

If your application uses [glm](https://github.com/g-truc/glm), define
`RCGP_SUPPORT_GLM` *before* including any RCGP header to enable implicit
conversions between RCGP's vector/matrix types and `glm::vecN` / `glm::matN`:

```cmake
target_compile_definitions(my_app PRIVATE RCGP_SUPPORT_GLM)
```

## Without CMake

If you build by hand, you need C++26 and the RCGP headers on your include
path:

```
-std=c++26 -I path/to/rcgp/include
```

You will also need to compile the sources under `path/to/rcgp/source/` (and
link the result into your binary alongside Vulkan, glslang, fmt, and glfw).
The CMake build is the supported path; the hand-rolled flags above are only
a sketch.
