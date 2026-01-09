#pragma once

#include <tuple>

// Tuples without the baggage
template <typename ... Args>
struct Tlist {
	static constexpr size_t size = sizeof...(Args);

	template <size_t I>
	using get = decltype([] {
		// GCC doesn't like indexing empty packs :(
		if constexpr (I < sizeof...(Args))
			return (Args...[I]) {};
		else
			return int();
	} ());

	template <typename T>
	using insert = Tlist <T, Args...>;

	template <template <typename ...> typename F>
	using invoke = F <Args...>;
};

// Static enumeration
template <size_t I>
using el = std::integral_constant <size_t, I>;

template <size_t ... Is>
auto el_series(std::index_sequence <Is...>)
{
	return std::tuple(el <Is> ()...);
}
