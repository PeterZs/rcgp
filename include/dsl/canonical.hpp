#pragma once

#include <type_traits>

#include "primitives_concepts.hpp"
#include "scalar.hpp"

namespace rcgp {

// Detect DSL scalar types
template <typename T>
struct is_scalar : std::false_type {};

template <native_scalar T>
struct is_scalar <scalar <T>> : std::true_type {};

template <typename T>
constexpr bool is_scalar_v = is_scalar <T> ::value;

// Canonical type: maps any scalar-like wrapper to its DSL scalar form
template <typename T>
struct canonical_type {
	using type = T;
};

template <native_scalar T>
struct canonical_type <T> {
	using type = scalar <T>;
};

// Common C++ arithmetic types that aren't native_scalar but should
// map to DSL scalars (e.g., double -> f32, int -> i32)
template <>
struct canonical_type <double> { using type = scalar <float>; };

template <>
struct canonical_type <int> { using type = scalar <int32_t>; };

template <>
struct canonical_type <unsigned int> { using type = scalar <uint32_t>; };

template <typename T>
using canonical_type_t = canonical_type <T> ::type;

// Runtime conversion to canonical form
template <typename T>
auto canonicalize(const T &value)
{
	return canonical_type_t <T> (value);
}

// Two types are canonically equivalent if they reduce to the same DSL type
template <typename T, typename U>
concept canonically_equivalent = std::same_as <
	canonical_type_t <T>,
	canonical_type_t <U>
>;

// A type is canonically scalar if its canonical form is a scalar<T>
template <typename T>
concept canonically_scalar = is_scalar_v <canonical_type_t <T>>;

// A type is canonically integral if its canonical form is scalar<int32_t> or scalar<uint32_t>
template <typename T>
concept canonically_integral = canonically_scalar <T>
	&& native_int_scalar <
		typename canonical_type_t <T> ::native_scalar_type
	>;

} // namespace rcgp
