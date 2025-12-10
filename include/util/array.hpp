#pragma once

#include <array>

template <typename T, size_t ... Ns>
auto concat(const std::array <T, Ns> &... arrays)
{
	std::array <T, (Ns + ...)> result {};
	size_t idx = 0;
	auto append = [&](const auto &arr) {
		for (const auto &el : arr)
			result[idx++] = el;
	};
	(append(arrays), ...);
	return result;
}
