#pragma once

#include "reconstruct_type.hpp"

namespace rcgp {

template <typename T>
jems::handle coerce_to_handle(const T &value)
{
	static_error(
		"unable to coerce value of type "_ss
		+ $ss_type(T)
		+ " into a jems::handle"_ss
	);
}

template <typename T>
requires std::is_base_of_v <jems::handle, T>
auto coerce_to_handle(const T &value)
{
	return value;
}

inline auto coerce_to_handle(std::nullptr_t value)
{
	return jems::handle();
}

template <aggregate T>
auto coerce_to_handle(const T &value)
{
	auto field_handler = [&] <size_t I> () {
		using field_t = typename T::fields::template get <I>;
		if constexpr (std::is_same_v <field_t, std::nullptr_t>) {
			return std::tuple <> ();
		} else {
			return std::tuple {
				coerce_to_handle(value
					.template _rcgp_get <I> ()
				)
			};
		}
	};

	auto args = constexpr_for(Is, T::field_count,
		return std::tuple_cat(
			field_handler.template operator() <Is> ()...
		)
	);

	return std::apply([&](auto &&... handles) {
		return jems::construct(reconstruct_type <T> (), handles...);
	}, args);
}

} // namespace rcgp
