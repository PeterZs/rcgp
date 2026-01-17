#pragma once

#include <array>
#include <functional>

#include "../util/array.hpp"
#include "../util/cti.hpp"
#include "reference.hpp"

template <auto & ... refs>
struct implicit_context {};

TYPE_TRAIT(is_implicit_context);
	template <auto &... refs>
	TYPE_TRAIT_INCLUDES(is_implicit_context, implicit_context <refs...>);

// TODO: simplify this with consteval functions
template <typename ... Ts>
struct find_implicit_context {
	static constexpr auto contexts = std::array { is_implicit_context_v <Ts>... };
	static constexpr auto idx = first_on(contexts);
	using type = Ts...[idx];
};

template <typename Ctx, typename ... Args>
struct filter_into_context {
	using type = Ctx;
};

template <auto &... refs, auto &ref, typename ... Args>
struct filter_into_context <implicit_context <refs...>, reference <ref>, Args...> {
	using next = implicit_context <refs..., ref>;
	using type = typename filter_into_context <next, Args...> ::type;
};

template <auto &... refs1, auto &... refs2, typename ... Args>
struct filter_into_context <implicit_context <refs1...>, implicit_context <refs2...>, Args...> {
	using next = implicit_context <refs1..., refs2...>;
	using type = typename filter_into_context <next, Args...> ::type;
};

template <typename Ctx, typename Arg, typename ... Args>
struct filter_into_context <Ctx, Arg, Args...> {
	using type = typename filter_into_context <Ctx, Args...> ::type;
};

// TODO: use void (__VA_ARGS__)
template <typename ... Args>
auto new_implicit_context(std::function <void (Args ...)>)
{
	using type = typename filter_into_context <implicit_context <>, Args...> ::type;
	return type();
}

#define $context_capture(...) (decltype(new_implicit_context(std::function([](__VA_ARGS__) {}))) _ref_context __VA_OPT__(,) __VA_ARGS__)
