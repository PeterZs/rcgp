compiler := "clang++"
cxxflags := "-std=c++26 -I include"

# Build RCGP code library
rcgp:
	cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug .
	cmake --build build -j

# Tests for scaffold generation
scaffold:
	{{compiler}} {{cxxflags}} -c tests/scaffold/std430.cpp -o /tmp/std430.o
	{{compiler}} {{cxxflags}} -c tests/scaffold/scalar.cpp -o /tmp/scalar.o

recording_src := "tests/dsl/recording.cpp"
recording_bin := "build/test_recording"

# Tests for DSL recording
recording: rcgp
	{{compiler}} {{cxxflags}} {{recording_src}} -o {{recording_bin}} -L build -l rcgp -l fmt
	./{{recording_bin}}

tracing_src := "tests/dsl/tracing.cpp"
tracing_bin := "build/test_tracing"

# Tests for shader and subroutine tracing
tracing: rcgp
	{{compiler}} {{cxxflags}} {{tracing_src}} -o {{tracing_bin}} -L build -l rcgp -l fmt
	./{{tracing_bin}}
