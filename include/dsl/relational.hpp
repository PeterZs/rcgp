#pragma once

#include "promotion.hpp"

namespace rcgp {

template <typename A, typename B>
concept dsl_equivalent_scalars =
	canonically_scalar <A>
	and canonically_scalar <B>
	and canonically_equivalent <A, B>
	and (is_scalar_v <A> or is_scalar_v <B>);

template <typename A, typename B>
concept dsl_ordered_scalars =
	dsl_equivalent_scalars <A, B>
	and (not std::is_same_v <typename canonical_type_t <A> ::native_scalar_type, bool>);

template <typename A, typename B>
requires dsl_equivalent_scalars <A, B>
scalar <bool> operator==(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eEqual, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires dsl_equivalent_scalars <A, B>
scalar <bool> operator!=(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eNotEqual, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires dsl_ordered_scalars <A, B>
scalar <bool> operator<(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLess, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires dsl_ordered_scalars <A, B>
scalar <bool> operator<=(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLessEqual, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires dsl_ordered_scalars <A, B>
scalar <bool> operator>(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eGreater, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires dsl_ordered_scalars <A, B>
scalar <bool> operator>=(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eGreaterEqual, canonicalize(a), canonicalize(b)));
}

} // namespace rcgp
