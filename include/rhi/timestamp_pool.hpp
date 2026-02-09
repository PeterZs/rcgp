#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace rcgp {

struct TimestampQueryResult {
	std::vector <uint64_t> stamps;
	vk::QueryResultFlags flags;
	double period;

	std::optional <double> delta(size_t a, size_t b) const;

	bool ready() const {
		return not stamps.empty();
	}
};

struct TimestampQueryPool {
	vk::QueryPool handle;
	vk::QueryResultFlags flags;
	double period;
	size_t count;
};

} // namespace rcgp
