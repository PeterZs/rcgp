#!/bin/sh
rm -rf build
export CC=clang CXX=clang++
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
cmake --build build -j 1 -t prototype
clang-build-analyzer --all build trace.txt
clang-build-analyzer --analyze trace.txt > analysis.txt
cat analysis.txt
