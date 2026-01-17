#pragma once

#include <stack>
#include <unordered_map>

#include "instructions.hpp"
#include "../util/logging.hpp"
#include "../util/runtime_type_registry.hpp"

struct Tracer {
	std::stack <SharedBlockReference> records;
	std::unordered_map <size_t, Reference> type_cache;

	Block &active() {
		if (records.empty())
			fatal("no active record");
		return *records.top();
	}

	static thread_local Tracer singleton;
};

inline thread_local Tracer Tracer::singleton;

#define $tsb Tracer::singleton.active()
