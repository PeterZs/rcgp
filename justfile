compiler := "clang++"
cxxflags := "-std=c++26 -I include"

# Build RCGP code library
build_rcgp:
	cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug .
	cmake --build build -j

# Build runtime tests
build_test: build_rcgp
	cmake --build build -j -t test

# Experimental program
experimental: build_rcgp
	cmake --build build -j -t experimental
	./build/experimental

# Compilation tests (static assertions, type checks)
compile:
	{{compiler}} {{cxxflags}} -c tests/compile/std430.cpp -o /tmp/std430.o
	{{compiler}} {{cxxflags}} -c tests/compile/scalar.cpp -o /tmp/scalar.o
	{{compiler}} {{cxxflags}} -c tests/compile/resources.cpp -o /tmp/resources.o
	{{compiler}} {{cxxflags}} -c tests/compile/canonical.cpp -o /tmp/canonical.o

# Tests for JIT tracing of the DSL
dsl *args: build_test
	./build/test {{args}}

# Run all tests
test: compile dsl
