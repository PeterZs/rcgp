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

# Compilation tests (static assertions, type checks, expected errors)
compile cxx=compiler:
	for t in std430 scalar resources canonical; do \
		{{cxx}} {{cxxflags}} -c tests/compile/$t.cpp -o /tmp/$t.o || exit 1; \
	done
	for t in tests/compile/shader_modules/*.cpp tests/compile/command_modules/*.cpp; do \
		python tests/compile/check_compile_fail.py {{cxx}} {{cxxflags}} $t || exit 1; \
	done

# Tests for JIT tracing of the DSL
dsl *args: build_test
	./build/test {{args}}

# Run all tests
test: compile dsl
