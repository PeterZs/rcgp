#pragma once

#include "scalar.hpp"

template <native_int_scalar T>
inline scalar <T> operator&(const scalar <T> &a, const scalar <T> &b)
{
	return scalar <T> ::reinterpret(jems::operation(OperationCode::eBitAnd, a, b));
}

template <native_int_scalar T>
inline scalar <T> operator|(const scalar <T> &a, const scalar <T> &b)
{
	return scalar <T> ::reinterpret(jems::operation(OperationCode::eBitOr, a, b));
}

template <native_int_scalar T>
inline scalar <T> operator^(const scalar <T> &a, const scalar <T> &b)
{
	return scalar <T> ::reinterpret(jems::operation(OperationCode::eBitXor, a, b));
}

template <native_int_scalar T>
inline scalar <T> operator~(const scalar <T> &v)
{
	return scalar <T> ::reinterpret(jems::operation(OperationCode::eBitNot, v, v));
}

template <native_int_scalar T>
inline scalar <T> operator<<(const scalar <T> &a, const scalar <T> &b)
{
	return scalar <T> ::reinterpret(jems::operation(OperationCode::eShiftLeft, a, b));
}

template <native_int_scalar T>
inline scalar <T> operator>>(const scalar <T> &a, const scalar <T> &b)
{
	return scalar <T> ::reinterpret(jems::operation(OperationCode::eShiftRight, a, b));
}
