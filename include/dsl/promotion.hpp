#pragma once

#include "canonical.hpp"

namespace rcgp {

// Scalar promotion rules (GLSL-style)
template <typename A, typename B>
struct scalar_promotion;

// Same type -> same type
template <native_scalar T>
struct scalar_promotion <scalar <T>, scalar <T>> {
	using type = scalar <T>;
};

// int32_t -> float
template <>
struct scalar_promotion <scalar <int32_t>, scalar <float>> {
	using type = scalar <float>;
};

template <>
struct scalar_promotion <scalar <float>, scalar <int32_t>> {
	using type = scalar <float>;
};

// uint32_t -> float
template <>
struct scalar_promotion <scalar <uint32_t>, scalar <float>> {
	using type = scalar <float>;
};

template <>
struct scalar_promotion <scalar <float>, scalar <uint32_t>> {
	using type = scalar <float>;
};

// Promoted scalar type from any two scalar-like types
template <typename A, typename B>
using promoted_scalar_t = scalar_promotion <
	canonical_type_t <A>,
	canonical_type_t <B>
> ::type;

// Two types are promotable if both are canonically scalar
// and a promotion rule exists between their canonical forms.
// At least one must be a DSL type to avoid hijacking native arithmetic.
template <typename A, typename B>
concept promotable_scalars =
	canonically_scalar <A>
	&& canonically_scalar <B>
	&& requires { typename promoted_scalar_t <A, B>; }
	&& (not (std::is_arithmetic_v <A> && std::is_arithmetic_v <B>));

// Convert a scalar-like value to a specific target scalar type
template <typename R, typename T>
R canonicalize_to(const T &value)
{
	return R(canonicalize(value));
}

} // namespace rcgp
