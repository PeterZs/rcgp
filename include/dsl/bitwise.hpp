#pragma once

#include "canonical.hpp"

namespace rcgp {

template <typename A, typename B>
requires canonically_equivalent <A, B> && canonically_integral <A>
	&& (is_scalar_v <A> or is_scalar_v <B>)
auto operator&(const A &a, const B &b)
{
	using R = canonical_type_t <A>;
	return R::reinterpret(jems::operation(OperationCode::eBitAnd, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires canonically_equivalent <A, B> && canonically_integral <A>
	&& (is_scalar_v <A> or is_scalar_v <B>)
auto operator|(const A &a, const B &b)
{
	using R = canonical_type_t <A>;
	return R::reinterpret(jems::operation(OperationCode::eBitOr, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires canonically_equivalent <A, B> && canonically_integral <A>
	&& (is_scalar_v <A> or is_scalar_v <B>)
auto operator^(const A &a, const B &b)
{
	using R = canonical_type_t <A>;
	return R::reinterpret(jems::operation(OperationCode::eBitXor, canonicalize(a), canonicalize(b)));
}

template <canonically_integral T>
auto operator~(const T &v)
{
	using R = canonical_type_t <T>;
	auto cv = canonicalize(v);
	return R::reinterpret(jems::operation(OperationCode::eBitNot, cv, cv));
}

template <typename A, typename B>
requires canonically_equivalent <A, B> && canonically_integral <A>
	&& (is_scalar_v <A> or is_scalar_v <B>)
auto operator<<(const A &a, const B &b)
{
	using R = canonical_type_t <A>;
	return R::reinterpret(jems::operation(OperationCode::eShiftLeft, canonicalize(a), canonicalize(b)));
}

template <typename A, typename B>
requires canonically_equivalent <A, B> && canonically_integral <A>
	&& (is_scalar_v <A> or is_scalar_v <B>)
auto operator>>(const A &a, const B &b)
{
	using R = canonical_type_t <A>;
	return R::reinterpret(jems::operation(OperationCode::eShiftRight, canonicalize(a), canonicalize(b)));
}

} // namespace rcgp
