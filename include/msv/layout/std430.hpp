#pragma once

#include <algorithm>
#include <array>
#include <type_traits>

#include "common.hpp"
#include "../../util/sequence.hpp"

template <typename T>
using is_unsized_array = std::bool_constant <
	std::is_array_v <T>
	&& std::extent_v <T> == 0
>;

namespace std430 {

// Compute inter-field padding for a fully static std430 tuple.
template <typename ... Ts>
consteval auto padding_for_static()
{
	constexpr size_t N = sizeof...(Ts);
	std::array <size_t, N> padding {};

	if constexpr (N == 0)
		return padding;

	constexpr auto sizes = std::array <size_t, N> { sizeof(Ts)... };
	constexpr auto aligns = std::array <size_t, N> { alignment_v <Ts> ... };

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
	constexpr auto aligns = std::array <size_t, N> { alignment_v <Ts> ... };

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
	constexpr auto aligns = std::array <size_t, N> { alignment_v <Ts> ... };

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
		static_assert(is_unsized_array <typename last_type <R> ::type> ::value, "dynamic member must be last");

		using dynamic_raw = typename last_type <R> ::type;
		using statics_raw = typename drop_last <R> ::type;

		using fixed_statics = decltype(fix_alignment(statics_raw {}));
		using dynamic_element = std::remove_extent_t <dynamic_raw>;

		static constexpr auto padding = padding_for_dynamic_from_seq <alignment_v <dynamic_element>> (fixed_statics {});

		using padding_seq = array_to_index_sequence <padding>;
		using stitched = layout_stitcher <fixed_statics, padding_seq> ::type;

		return decltype(new_dynamic_tuple(dynamic_element {}, stitched {})) ();
	} else {
		static_assert(![] { return (is_unsized_array <Ts> ::value || ...); } (), "dynamic member must be last");

		static constexpr auto padding = padding_for_static <Ts...> ();

		using field_seq = sequence <Ts...>;
		using padding_seq = array_to_index_sequence <padding>;

		using stitched = layout_stitcher <field_seq, padding_seq> ::type;

		return typename stitched::trivial_tuple();
	}
}

template <typename ... Ts>
consteval auto layout_engine(sequence <Ts...> seq)
{
	using fseq = decltype(fix_alignment(seq));
	using tuple = decltype(layout_engine_dispatch(std::declval <fseq> ()));
	return tuple();
}

} // namespace std430

template <typename ... Ts>
using std430_layout_t = decltype(std430::layout_engine(std::declval <sequence <Ts...>> ()));
