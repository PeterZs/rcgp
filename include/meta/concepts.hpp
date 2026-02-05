#pragma once

#include <type_traits>

#include "../dsl/array.hpp"
#include "../util/cti.hpp"

namespace rcgp {

// Primitive types
template <typename T>
concept builtin = std::is_base_of_v <jems::handle, T>;

// User-defined types
template <typename T>
concept user_defined = requires {
	typename T::_rcgp_user_defined;
} && std::is_same_v <
	typename T::_rcgp_user_defined,
	std::type_identity <T>
>;

// Dynamic types
TYPE_TRAIT(is_dynamic);
	template <typename T>
	TYPE_TRAIT_INCLUDES(is_dynamic, array <T, -1>);

template <user_defined T>
struct is_dynamic <T> {
	static constexpr bool value = [] {
		return constexpr_for(Is, T::field_count,
			return (is_dynamic <
				typename T::fields::template get <Is>
			> ::value || ...)
		);
	} ();
};

template <typename T>
concept dynamic = is_dynamic_v <T>;

template <typename T>
concept non_dynamic = not is_dynamic_v <T>;

// Anything (builtin or user_defined) that is traced in our JIT system
template <typename T>
concept traced = requires (T &value, const Reference &ref) {
	{ value.override_reference(ref) };
};

} // namespace rcgp
