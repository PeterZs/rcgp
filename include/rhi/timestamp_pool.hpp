#pragma once

#include "../util/logging.hpp"

struct TimestampQueryResult {
	std::vector <uint64_t> stamps;
	vk::QueryResultFlags flags;
	double period;

	std::optional <double> delta(size_t a, size_t b) const {
		assertion(ready(), "cannot query deltas for unready time stamps");

		auto K = period / 1'000'000.0f;
		if (flags & vk::QueryResultFlagBits::eWithAvailability) {
			if (stamps[2 * a + 1] != 0 && stamps[2 * b - 1] != 0)
				return double(stamps[2 * b] - stamps[2 * a]) * K;
			else
				return std::nullopt;
		} else {
			return double(stamps[b] - stamps[a]) * K;
		}
	}

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
