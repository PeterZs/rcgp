#pragma once

#include <cstdlib>
#include <unordered_map>
#include <vector>

#include "instruction_nodes.hpp"

namespace rcgp {

struct Tracer {
	std::vector <SharedBlockReference> records;
	std::unordered_map <std::string, Reference> type_cache;

	Block &active() {
		if (records.empty())
			std::abort();
		return *records.back();
	}

	static thread_local Tracer singleton;
};

inline thread_local Tracer Tracer::singleton;

#define $tsb Tracer::singleton.active()

} // namespace rcgp
