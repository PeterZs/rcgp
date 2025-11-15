#pragma once

#include "../dsl/jems.hpp"
#include "../dsl/primitives.hpp"
#include "reflection.hpp"
#include "reflection_expander.hpp"
#include "static_string.hpp"

template <typename T>
struct reconstructor_t {
	static jems::handle main() {
		static_assert(false, ($ss("type reconstructor for ") + $ss_type(T) + $ss(" has not been implemented yet")).view());
	}
};

template <typename T, size_t N>
struct reconstructor_t <vector <T, N>> {
	static jems::handle main() {
		return jems::type(VectorType <T, N> ());
	}
};

template <typename T, size_t N, size_t M>
struct reconstructor_t <matrix <T, N, M>> {
	static jems::handle main() {
		return jems::type(MatrixType <T, N, M> ());
	}
};

template <typename Original, typename ... Args>
struct reconstructor_t <aggregate_reflection <Original, Args...>> {
	static bool collect(AggregateType &aggregate, jems::handle handle) {
		aggregate.push_back(handle.ref);
		return true;
	}

	static jems::handle main() {
		fmt::println("reconstructor for {}", $ss_type(Original).view());

		AggregateType aggregate;
		std::apply([&](auto ... xs) {
			std::make_tuple(collect(
				aggregate,
				reconstructor_t <decltype(xs)> ::main()
			)...);
		}, std::tuple <Args...> ());

		return jems::type(aggregate);
	}
};

template <typename T>
jems::handle reconstruct_type()
{
	using R = reflection_expander <T> ::type;
	return reconstructor_t <R> ::main();
}
