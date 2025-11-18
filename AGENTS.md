# Repository Guidelines

## Project Structure & Module Organization
The root hosts `CMakeLists.txt`, `build-analysis.sh`, and the generated `build/` tree. Engine headers live in `include/` with submodules: `dsl/` for shader DSL primitives, `msv/` for runtime reflection/injection utilities, `rhi/` for Vulkan device/session glue, and `util/` for helpers. Sample applications under `samples/` illustrate usage (`prototype.cpp`, `traditional.cpp`). Third-party mirrors live in `ext/` (`glm`, `glslang`) and should not be modified without upstream sync.

## Build, Test, and Development Commands
Run `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON` after cloning to configure a Clang-based build. Invoke `cmake --build build -j` for all targets or `cmake --build build -j -t prototype` / `-t traditional` for focused iteration. Execute samples directly from `./build/prototype` etc. `./build-analysis.sh` rebuilds with tracing enabled and feeds results to `clang-build-analyzer` for hotspot diagnostics. Use `ninja` or `make` generators interchangeably as long as the `build/` directory stays isolated.

## Coding Style & Naming Conventions
Code targets C++26 with Clang; include headers via `<ugp.hpp>` and keep DSL macros (`$vertex`, `$fn`, `$returns`) on separate lines for readability. Prefer PascalCase for structs/types (`RasterForward`), snake_case for functions and variables (`inject_execution_model`, `transforms`). Keep public headers self-contained and guarded with `#pragma once`. Align template-heavy expressions using tabs already present in the file, and avoid trailing whitespace.

## Commit Guidelines
Commits follow short, imperative subjects (see `git log`: “Vector swizzling with zero storage overhead”). Keep body text for rationale and mention affected subsystems (`dsl`, `msv`, etc.). Document expected assembly or performance impacts inside the commit message when touching the DSL or generators so future contributors can understand the regression baseline.
