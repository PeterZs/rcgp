#pragma once

#include <cstddef>

namespace rcgp {

// NOTE: runtime type IDs without RTTI; reset only on process restart.
struct RuntimeTypeRegistry {
	static inline size_t counter = 0;

	template <typename T>
	static size_t id() {
		static size_t value = counter++;
		return value;
	}
};

} // namespace rcgp
