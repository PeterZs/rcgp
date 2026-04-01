#pragma once

#include "promotion.hpp"
#include "vector.hpp"
#include "matrix.hpp"

namespace rcgp {

// Cross-type scalar arithmetic via promotion.
// Same-canonical-type ops (e.g. swizzle<u32> * u32) go through the friend
// operators in scalar.hpp via implicit conversion. These free functions
// only handle cases where the canonical types DIFFER (e.g. f32 + i32).
template <typename A, typename B>
requires promotable_scalars <A, B>
	&& (not canonically_equivalent <A, B>)
auto operator+(const A &a, const B &b)
{
	using R = promoted_scalar_t <A, B>;
	return R::reinterpret(jems::operation(OperationCode::eAdd,
		canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires promotable_scalars <A, B>
	&& (not canonically_equivalent <A, B>)
auto operator-(const A &a, const B &b)
{
	using R = promoted_scalar_t <A, B>;
	return R::reinterpret(jems::operation(OperationCode::eSubtract,
		canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires promotable_scalars <A, B>
	&& (not canonically_equivalent <A, B>)
auto operator*(const A &a, const B &b)
{
	using R = promoted_scalar_t <A, B>;
	return R::reinterpret(jems::operation(OperationCode::eMultiply,
		canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires promotable_scalars <A, B>
	&& (not canonically_equivalent <A, B>)
auto operator/(const A &a, const B &b)
{
	using R = promoted_scalar_t <A, B>;
	return R::reinterpret(jems::operation(OperationCode::eDivide,
		canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires promotable_scalars <A, B>
	&& canonically_integral <A>
	&& (not canonically_equivalent <A, B>)
auto operator%(const A &a, const B &b)
{
	using R = promoted_scalar_t <A, B>;
	return R::reinterpret(jems::operation(OperationCode::eMod,
		canonicalize(a), canonicalize(b)));
}

// Matrix-vector multiplication
template <native_scalar T, size_t N, size_t M>
vector <T, M> operator*(const matrix <T, N, M> &m, const vector <T, N> &v)
{
	return vector <T, M> ::reinterpret(jems::operation(OperationCode::eMultiply, m, v));
}

// Matrix-matrix multiplication
template <native_scalar T, size_t N, size_t M, size_t K>
matrix <T, N, M> operator*(const matrix <T, N, K> &a, const matrix <T, K, M> &b)
{
	return matrix <T, N, M> ::reinterpret(jems::operation(OperationCode::eMultiply, a, b));
}

} // namespace rcgp
