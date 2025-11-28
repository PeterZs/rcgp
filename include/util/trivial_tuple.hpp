#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

// Trivially constructable tuple that supports POD fields and dynamic arrays
template <typename ... Ts>
class trivial_tuple {};

template <>
class trivial_tuple <> {
public:
	template <size_t Index>
	static consteval size_t offset() {
		static_assert(Index != Index, "tuple index out of range");
		return 0;
	}
};

template <typename T, typename ... Ts>
class trivial_tuple <T, Ts...> {
	T x;
	[[no_unique_address]] trivial_tuple <Ts...> rest;
public:
	template <size_t Index>
	auto &get() {
		if constexpr (Index == 0) {
			return x;
		} else {
			return rest.template get <Index - 1> ();
		}
	}

	template <size_t Index>
	const auto &get() const {
		if constexpr (Index == 0) {
			return x;
		} else {
			return rest.template get <Index - 1> ();
		}
	}

	// Multi-level indices
	template <size_t Index, size_t ... Rest>
	auto &get_recursive() {
		auto &value = get <Index> ();
		if constexpr (sizeof...(Rest))
			return value.template get <Rest...> ();
		else
			return value;
	}

	template <size_t Index, size_t ... Rest>
	const auto &get_recursive() const {
		const auto &value = get <Index> ();
		if constexpr (sizeof...(Rest))
			return value.template get <Rest...> ();
		else
			return value;
	}

	template <size_t Index>
	static consteval size_t offset() {
		if constexpr (Index == 0) {
			return 0;
		} else {
			return sizeof(T) + trivial_tuple <Ts...> ::template offset <Index - 1> ();
		}
	}
};

template <typename Tuple, size_t ... Is>
using element_t = std::decay_t <
	decltype(std::declval <Tuple> ().template get_recursive <Is...> ())
>;
