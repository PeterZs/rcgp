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

// Tlist filtering with arbitrary conditions
template <template <typename> typename Filter, typename ... Ts>
auto tlist_filter(Tlist <Ts...> ongoing, Tlist <> processing)
{
	return ongoing;
}

template <template <typename> typename Filter, typename ... Ts, typename C, typename ... Rest>
auto tlist_filter(Tlist <Ts...> ongoing, Tlist <C, Rest...> processing)
{
	constexpr auto rest = Tlist <Rest...> {};
	if constexpr (Filter <C> ::value)
		return tlist_filter <Filter> (Tlist <Ts..., C> {}, rest);
	else
		return tlist_filter <Filter> (ongoing, rest);
}

template <template <typename> typename Filter, typename List>
using tlist_filter_t = decltype(tlist_filter <Filter> (Tlist <> {}, List {}));
