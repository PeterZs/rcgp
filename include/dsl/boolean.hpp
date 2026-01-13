#pragma once

#include "scalar.hpp"

inline scalar <bool> operator&&(const scalar <bool> &a, const scalar <bool> &b) {
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLogicalAnd, a, b));
}

inline scalar <bool> operator||(const scalar <bool> &a, const scalar <bool> &b) {
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLogicalOr, a, b));
}

inline scalar <bool> operator^(const scalar <bool> &a, const scalar <bool> &b) {
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLogicalXor, a, b));
}

inline scalar <bool> operator!(const scalar <bool> &v) {
	return scalar <bool> ::reinterpret(jems::operation(OperationCode::eLogicalNot, v, v));
}
