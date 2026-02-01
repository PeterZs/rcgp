#pragma once

#include <cstdlib>
#include <iostream>
#include <print>
#include <stack>
#include <unordered_map>

#include "instructions.hpp"
#include "../util/runtime_type_registry.hpp"

namespace rcgp {

struct Tracer {
	std::stack <SharedBlockReference> records;
	std::unordered_map <size_t, Reference> type_cache;

	Block &active() {
		if (records.empty()) {
			std::println(std::cerr, "no active record");
			std::abort();
		}
		return *records.top();
	}

	static thread_local Tracer singleton;
};

inline thread_local Tracer Tracer::singleton;

#define $tsb Tracer::singleton.active()

} // namespace rcgp
