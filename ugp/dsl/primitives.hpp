#pragma once

#include <cstdlib>
#include <stdfloat>
#include <type_traits>

#include "jems.hpp"

template <typename T>
struct scalar {
	static_assert(std::is_arithmetic_v <T>);
};

using i32 = scalar <int32_t>;

template <typename T, size_t N>
struct vector_base {};

template <typename T>
struct vector_base <T, 4> : jems::handle {
	vector_base() = default;

	vector_base(const vector_base <T, 2> &xy, const T &z, const T &w, $location)
		: handle(jems::construct_loc(loc,
			jems::type_loc(loc, VectorType <T, 4> ()),
			jems::constant_loc(loc, z),
			jems::constant_loc(loc, w)
		)) {}
};

template <typename T, size_t N>
struct vector : vector_base <T, N> {
	using vector_base <T, N> ::vector_base;

	static_assert(std::is_arithmetic_v <T>);
};

// TODO: float32_t
using vec2 = vector <float, 2>;
using vec3 = vector <float, 3>;
using vec4 = vector <float, 4>;

using ivec3 = vector <int32_t, 3>;

template <typename T, size_t N, size_t M>
struct matrix {
	static_assert(std::is_arithmetic_v <T>);
};

using mat4 = matrix <float, 4, 4>;
