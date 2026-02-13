#pragma once

#include <cstddef>
#include <tuple>

namespace rcgp {

template <typename... Ts>
struct PNextChain : std::tuple <Ts...> {
	static_assert(sizeof...(Ts) > 0);

	constexpr PNextChain() {
		setup_chain <0> ();
	}

	template <size_t I>
	auto &get() {
		return std::get <I> (*this);
	}

	template <size_t I>
	auto const &get() const {
		return std::get <I> (*this);
	}
private:
	template <size_t I>
	constexpr void setup_chain() {
		auto &current = std::get <I> (*this);
		if constexpr (I + 1 < sizeof...(Ts)) {
			current.pNext = &std::get <I + 1> (*this);
			setup_chain <I + 1> ();
		} else {
			current.pNext = nullptr;
		}
	}
};

} // namespace rcgp
