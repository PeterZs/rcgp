#pragma once

#include "enumerations.hpp"

namespace rcgp {

template <typename T, size_t N = 0, size_t M = 0>
consteval PrimitiveType primitive_of()
{
	if constexpr (N == 0 && M == 0) {
		if constexpr (std::is_same_v <T, bool>)
			return PrimitiveType::eBool;
		else if constexpr (std::is_same_v <T, int32_t>)
			return PrimitiveType::eInt32;
		else if constexpr (std::is_same_v <T, uint32_t>)
			return PrimitiveType::eUInt32;
		else if constexpr (std::is_same_v <T, float>)
			return PrimitiveType::eFloat;
		else
			static_assert(false, "unsupported scalar primitive");
	} else if constexpr (M == 0) {
		static_assert(N >= 2 && N <= 4, "unsupported vector dimension");
		if constexpr (std::is_same_v <T, uint32_t>) {
			return static_cast <PrimitiveType> (
				static_cast <int> (PrimitiveType::eUVec2) + static_cast <int> (N - 2)
			);
		} else if constexpr (std::is_same_v <T, int32_t>) {
			return static_cast <PrimitiveType> (
				static_cast <int> (PrimitiveType::eIVec2) + static_cast <int> (N - 2)
			);
		} else if constexpr (std::is_same_v <T, float>) {
			return static_cast <PrimitiveType> (
				static_cast <int> (PrimitiveType::eVec2) + static_cast <int> (N - 2)
			);
		} else {
			static_assert(false, "unsupported vector primitive");
		}
	} else {
		static_assert(N >= 2 && N <= 4, "unsupported matrix dimension");
		static_assert(M >= 2 && M <= 4, "unsupported matrix dimension");
		constexpr int index = static_cast <int> ((N - 2) * 3 + (M - 2));
		if constexpr (std::is_same_v <T, int32_t>) {
			return static_cast <PrimitiveType> (
				static_cast <int> (PrimitiveType::eIMat2x2) + index
			);
		} else if constexpr (std::is_same_v <T, uint32_t>) {
			return static_cast <PrimitiveType> (
				static_cast <int> (PrimitiveType::eUMat2x2) + index
			);
		} else if constexpr (std::is_same_v <T, float>) {
			return static_cast <PrimitiveType> (
				static_cast <int> (PrimitiveType::eFMat2x2) + index
			);
		} else {
			static_assert(false, "unsupported matrix primitive");
		}
	}
}

} // namespace rcgp
