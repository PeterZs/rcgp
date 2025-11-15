#pragma once

template <typename T>
struct reflection_expander {
	using type = decltype([] {
		if constexpr (has_reflection <T> ())
			// TODO: needs to be recursive...
			return typename T::reflection {};
		else
			return T {};
	} ());
};
