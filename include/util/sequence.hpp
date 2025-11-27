#pragma once

#include <tuple>

#include "trivial_tuple.hpp"

// Tuples without the backage
template <typename ... Args>
struct sequence {
	sequence() = default;

	template <typename T, typename ... Rest>
	sequence(T, Rest...) {}

	template <typename T>
	using push_front_t = sequence <T, Args...>;

	template <typename T>
	using push_back_t = sequence <Args..., T>;

	using tuple = std::tuple <Args...>;
	using trivial_tuple = trivial_tuple <Args...>;
};

template <typename Seq>
struct last_type;

template <typename T>
struct last_type <sequence <T>> {
	using type = T;
};

template <typename T, typename ... Ts>
struct last_type <sequence <T, Ts...>> {
	using type = typename last_type <sequence <Ts...>> ::type;
};

template <typename Seq>
struct drop_last;

template <typename T>
struct drop_last <sequence <T>> {
	using type = sequence <>;
};

template <typename T, typename ... Ts>
struct drop_last <sequence <T, Ts...>> {
	using tail = typename drop_last <sequence <Ts...>> ::type;
	using type = typename tail::template push_front_t <T>;
};

// Static enumeration
template <size_t I>
using el = std::integral_constant <size_t, I>;

template <size_t ... Is>
auto el_series(std::index_sequence <Is...>)
{
	return std::tuple(el <Is> ()...);
}
