#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>

#include <glm/glm.hpp>

#include "../../dsl/primitives.hpp"
#include "../../util/align.hpp"
#include "../../util/dynamic_tuple.hpp"
#include "../../util/sequence.hpp"
#include "common.hpp"

template <typename T>
using is_unsized_array = std::bool_constant <
	std::is_array_v <T>
	&& std::extent_v <T> == 0
>;

namespace std430::detail {

template <typename T>
struct alignment {};

// TODO: by byte size...
template <native_scalar T>
struct alignment <T> {
	static constexpr size_t value = 4;
};

template <typename T>
struct alignment <glm::tvec2 <T, glm::qualifier::defaultp>> {
	static constexpr size_t value = 8;
};

template <typename T>
struct alignment <glm::tvec3 <T, glm::qualifier::defaultp>> {
	static constexpr size_t value = 16;
};

template <typename T, size_t N>
struct alignment <T[N]> {
	static constexpr size_t value = alignment <T> ::value;
};

template <typename T>
struct alignment <T[]> {
	static constexpr size_t value = alignment <T> ::value;
};

template <typename T, size_t N>
struct alignment <padded_t <T, N>> {
	static constexpr size_t value = alignment <T> ::value;
};

template <typename T>
constexpr size_t alignment_v = alignment <T> ::value;

} // namespace std430::detail

namespace std430 {

template <typename T>
consteval size_t std430_alignof();

template <typename T>
struct padded_base {
	static constexpr bool value = false;
};

template <typename U, size_t Pad>
struct padded_base <padded_t <U, Pad>> {
	using type = U;
	static constexpr bool value = true;
};

template <typename T>
struct trivial_tuple_args {};

template <typename ... Ts>
struct trivial_tuple_args <trivial_tuple <Ts...>> {
	static consteval size_t align() {
		if constexpr (sizeof...(Ts) == 0)
			return 1;
		else
			return std::max({ std430_alignof <Ts> () ... });
	}
};

template <typename T>
struct dynamic_tuple_args {};

template <typename T, typename ... Ts>
struct dynamic_tuple_args <dynamic_tuple <T, Ts...>> {
	static consteval size_t align() {
		return std::max({
			std430_alignof <T> (),
			std430_alignof <Ts> ()...
		});
	}
};

template <typename ... Ts>
consteval auto layout_engine(sequence <Ts...>);

template <typename T>
consteval size_t std430_alignof()
{
	if constexpr (std::is_array_v <T>) {
		return std430_alignof <std::remove_extent_t <T>> ();
	} else if constexpr (requires { detail::alignment <T> ::value; }) {
		return detail::alignment <T> ::value;
	} else if constexpr (padded_base <T> ::value) {
		using base = typename padded_base <T> ::type;
		return std430_alignof <base> ();
	} else if constexpr (requires { trivial_tuple_args <T> ::align(); }) {
		return trivial_tuple_args <T> ::align();
	} else if constexpr (requires { dynamic_tuple_args <T> ::align(); }) {
		return dynamic_tuple_args <T> ::align();
	} else {
		return alignof(T);
	}
}

namespace detail {

template <typename ... Ts>
struct alignment <trivial_tuple <Ts...>> {
	static constexpr size_t value = [] {
		if constexpr (sizeof...(Ts) == 0)
			return size_t(1);
		else
			return std::max({ std430_alignof <Ts> () ... });
	} ();
};

} // namespace detail

template <typename T>
struct computed_layout_type {
	using type = T;
};

template <typename T, size_t N>
struct computed_layout_type <T[N]> {
	using element = typename computed_layout_type <T> ::type;

	static constexpr size_t stride = align_up(sizeof(element), std430_alignof <element> ());
	using padded = padded_t <element, stride - sizeof(element)>;

	using type = padded[N];
};

template <typename T>
struct computed_layout_type <T[]> {
	using element = typename computed_layout_type <T> ::type;

	static constexpr size_t stride = align_up(sizeof(element), std430_alignof <element> ());
	using padded = padded_t <element, stride - sizeof(element)>;

	using type = padded[];
};

template <typename ... Ts>
struct computed_layout_type <sequence <Ts...>> {
	using type = decltype(layout_engine(sequence <Ts...> ::singleton));
};

// Compute inter-field padding for a fully static std430 tuple.
template <typename ... Ts>
consteval auto padding_for_static()
{
	constexpr size_t N = sizeof...(Ts);
	std::array <size_t, N> padding {};

	if constexpr (N == 0)
		return padding;

	constexpr auto sizes = std::array <size_t, N> { sizeof(Ts)... };
	constexpr auto aligns = std::array <size_t, N> { std430_alignof <Ts> () ... };

	size_t offset = 0;
	size_t malign = 0;

	for (size_t i = 0; i < N; i++) {
		auto corrected = align_up(offset, aligns[i]);

		if (i > 0)
			padding[i - 1] = corrected - offset;

		offset = corrected + sizes[i];
		malign = std::max(malign, aligns[i]);
	}

	padding[N - 1] = align_up(offset, malign) - offset;

	return padding;
}

// Compute padding for statics when a trailing dynamic array needs alignment.
template <size_t AlignDynamic, typename ... Ts>
consteval auto padding_for_dynamic()
{
	constexpr size_t N = sizeof...(Ts);
	std::array <size_t, N> padding {};

	if constexpr (N == 0)
		return padding;

	constexpr auto sizes = std::array <size_t, N> { sizeof(Ts)... };
	constexpr auto aligns = std::array <size_t, N> { std430_alignof <Ts> () ... };

	size_t offset = 0;
	size_t malign = 0;

	for (size_t i = 0; i < N; i++) {
		auto corrected = align_up(offset, aligns[i]);

		if (i > 0)
			padding[i - 1] = corrected - offset;

		offset = corrected + sizes[i];
		malign = std::max(malign, aligns[i]);
	}

	size_t target_align = std::max(malign, AlignDynamic);
	padding[N - 1] = align_up(offset, target_align) - offset;

	return padding;
}

// Sequence-friendly wrapper around padding_for_dynamic.
template <size_t AlignDynamic, typename ... Ts>
consteval auto padding_for_dynamic_from_seq(sequence <Ts...>)
{
	return padding_for_dynamic <AlignDynamic, Ts...> ();
}

// Legacy fallback layout path for static tuples.
template <typename ... Ts>
consteval auto layout_engine_impl()
{
	constexpr size_t N = sizeof...(Ts);

	constexpr auto sizes = std::array <size_t, N> { sizeof(Ts)... };
	constexpr auto aligns = std::array <size_t, N> { detail::alignment_v <Ts> ... };

	std::array <size_t, N> padding {};

	size_t offset = 0;
	size_t malign = 0;

	for (size_t i = 0; i < N; i++) {
		auto corrected = align_up(offset, aligns[i]);

		if (i > 0)
			padding[i - 1] = corrected - offset;

		offset = corrected + sizes[i];
		malign = std::max(malign, aligns[i]);
	}

	padding[N - 1] = align_up(offset, malign) - offset;

	return padding;
}

// Entry point; stitch std430 tuples, emitting dynamic_tuple when a trailing T[] is present.
template <typename ... Ts>
consteval auto layout_engine_dispatch(sequence <Ts...>)
{
	using R = sequence <Ts...>;
	using L = typename last_type <R> ::type;
	constexpr bool has_dynamic = is_unsized_array <L> ::value;

	if constexpr (has_dynamic) {
		// TODO: assert that the other elements arent dynamic (sequence <Ts...> ::logical_or <condition>)
		// static_assert(is_unsized_array <L> ::value, "dynamic member must be last");

		using statics_raw = typename drop_last <R> ::type;

		using dynamic_element = std::remove_reference_t <L>;

		static constexpr auto padding = padding_for_dynamic_from_seq
			<std430_alignof <dynamic_element> ()> (statics_raw::singleton);

		using padding_seq = array_to_index_sequence <padding>;
		using stitched = layout_stitcher <statics_raw, padding_seq> ::type;

		return decltype(new_dynamic_tuple(std::type_identity <dynamic_element> {}, stitched::singleton)) ();
	} else {
		static constexpr auto any_unsized = [] {
			return (is_unsized_array <Ts> ::value || ...);
		} ();

		static_assert(!any_unsized, "dynamic member must be last");

		static constexpr auto padding = padding_for_static <Ts...> ();

		using field_seq = sequence <Ts...>;
		using padding_seq = array_to_index_sequence <padding>;

		using stitched = layout_stitcher <field_seq, padding_seq> ::type;

		return typename stitched::trivial_tuple();
	}
}

template <typename ... Ts>
consteval auto layout_engine(sequence <Ts...>)
{
	using computed = sequence <typename computed_layout_type <Ts> ::type...>;
	using tuple = decltype(layout_engine_dispatch(computed::singleton));
	return tuple();
}

} // namespace std430
