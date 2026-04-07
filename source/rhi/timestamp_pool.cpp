#include "rhi/timestamp_pool.hpp"
#include "util/error.hpp"

namespace rcgp {

std::optional <double> TimestampQueryResult::delta(size_t a, size_t b) const
{
	if (!ready())
		fatal("cannot query deltas for unready timestamps");

	auto K = period / 1'000'000.0f;
	if (flags & vk::QueryResultFlagBits::eWithAvailability) {
		if (stamps[2 * a + 1] != 0 && stamps[2 * b - 1] != 0)
			return double(stamps[2 * b] - stamps[2 * a]) * K;
		return std::nullopt;
	}

	return double(stamps[b] - stamps[a]) * K;
}

} // namespace rcgp
