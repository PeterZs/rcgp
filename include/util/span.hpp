#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <vector>
#include <type_traits>

template <typename T, std::size_t N = std::dynamic_extent>
struct span : std::span <T, N> {
	using std::span <T, N> ::span;

	// Map each element into a new std::vector when the extent is dynamic.
	template <typename F>
	auto map(const F &ftn)
	requires (N == std::dynamic_extent) {
		using R = std::invoke_result_t <F, T>;

		size_t size = this->size();
		std::vector <R> result(size);
		for (size_t i = 0; i < size; i++)
			result[i] = ftn(this->operator[](i));

		return result;
	}
};

// Deduction guides so CTAD works with C-arrays, std::array, std::vector and similar.
template <typename T, std::size_t N>
span(T (&)[N]) -> span <T, N>;

template <typename T, std::size_t N>
span(std::array <T, N> &) -> span <T, N>;

template <typename T, std::size_t N>
span(const std::array <T, N> &) -> span <const T, N>;

template <typename R>
span(R &) -> span <typename R::value_type>;

template <typename R>
span(const R &) -> span <const typename R::value_type>;
