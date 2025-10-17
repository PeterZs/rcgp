#pragma once

#include <stack>

#include <fmt/printf.h>

#include "instructions.hpp"

using record_base = std::vector <Instruction>;

// NOTE: this tracer is not limited to just shaders...
// it also includes game engine stuff/code
struct Tracer {
	struct Record : record_base {
		template <typename ... Args>
		Index add(Args ... args) {
			auto result = size();
			emplace_back(args...);
			return result;
		}

		Instruction &operator[](const Index &a) {
			return record_base::operator[](a);
		}
		
		const Instruction &operator[](const Index &a) const {
			return record_base::operator[](a);
		}
	};

	std::stack <std::reference_wrapper <Record>> records;

	Record &active() {
		if (records.empty()) {
			fmt::println(stderr, "no active record");
			__builtin_trap();
		}

		return records.top();
	}

	// Singleton
	static thread_local Tracer singleton;
};

inline thread_local Tracer Tracer::singleton;
