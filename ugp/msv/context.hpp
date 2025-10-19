#pragma once

#include <functional>

#include "reference.hpp"

template <auto & ... refs>
struct reference_context {};

template <auto & ... refs>
auto new_reference_context_impl(reference_context <refs...>) -> reference_context <refs...>;

template <typename T, typename ... Args, auto &... refs>
auto new_reference_context_impl(reference_context <refs...> context, T *, Args *... args)
{
	if constexpr (is_reference <T> ::value) {
		return new_reference_context_impl <Args...> (reference_context <refs ..., T::handle> (), args...);
	} else {
		return new_reference_context_impl <Args...> (context, args...);
	}
}

template <typename ... Args>
auto new_reference_context(std::function <void (Args ...)>)
{
	return new_reference_context_impl <Args...> (reference_context <> (), (Args *) nullptr...);
}

#define $context_capture(...) (decltype(new_reference_context(std::function([](__VA_ARGS__) {}))) _ref_context __VA_OPT__(,) __VA_ARGS__)
#define $context _ref_context
