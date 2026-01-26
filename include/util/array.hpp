#pragma once

#include <array>

namespace rcgp {

template <size_t N>
constexpr int64_t first_on(const std::array <bool, N> &x)
{
	for (size_t i = 0; i < N; i++)
		if (x[i]) return i;
	return -1;
}

} // namespace rcgp
