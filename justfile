compiler := "clang++"
cxxflags := "-std=c++26 -I include"

rcgp:
	cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug .
	cmake --build build -j

scaffold:
	{{compiler}} {{cxxflags}} -c tests/scaffold/std430.cpp -o /tmp/std430.o
	{{compiler}} {{cxxflags}} -c tests/scaffold/scalar.cpp -o /tmp/scalar.o

recording_src := "tests/dsl/recording.cpp"
recording_bin := "build/test_recording"
recording: rcgp
	if [ ! -f {{recording_bin}} ] || [ {{recording_src}} -nt {{recording_bin}} ]; then \
		{{compiler}} {{cxxflags}} {{recording_src}} -o {{recording_bin}} -L build -l rcgp -l fmt; \
	fi
	./{{recording_bin}}
