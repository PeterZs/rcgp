#pragma once

#include <type_traits>

#include "projection.hpp"

namespace rcgp {

template <typename A, typename B>
concept projectively_equivalent_dsl_scalar =
	projectively_scalar <A>
	and projectively_scalar <B>
	and projectively_equivalent <A, B>
	and (is_scalar_v <A> or is_scalar_v <B>);

template <typename A, typename B>
concept projectively_ordered_scalar =
	projectively_equivalent_dsl_scalar <A, B>
	and (not std::is_same_v <typename projection_t <A> ::native_scalar_type, bool>);

template <typename A, typename B>
requires projectively_equivalent_dsl_scalar <A, B>
scalar <bool> operator==(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eEqual, project(a), project(b)));
}

template <typename A, typename B>
requires projectively_equivalent_dsl_scalar <A, B>
scalar <bool> operator!=(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eNotEqual, project(a), project(b)));
}

template <typename A, typename B>
requires projectively_ordered_scalar <A, B>
scalar <bool> operator<(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLess, project(a), project(b)));
}

template <typename A, typename B>
requires projectively_ordered_scalar <A, B>
scalar <bool> operator<=(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLessEqual, project(a), project(b)));
}

template <typename A, typename B>
requires projectively_ordered_scalar <A, B>
scalar <bool> operator>(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eGreater, project(a), project(b)));
}

template <typename A, typename B>
requires projectively_ordered_scalar <A, B>
scalar <bool> operator>=(const A &a, const B &b)
{
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eGreaterEqual, project(a), project(b)));
}

} // namespace rcgp
