#pragma once

#include "reflection.hpp"

// Scaffold for primitive/built-in types
template <size_t Align, typename T>
struct alignas(Align) scaffold_fundamental {
	T value;

	// TODO: constructors and operators...
};

template <size_t Align, typename T>
struct alignas(Align) scaffold_structural : T {
	constexpr scaffold_structural() = default;
	constexpr scaffold_structural(const T &rhs) : T(rhs) {}
	constexpr scaffold_structural &operator=(const T &rhs) {
		T::operator=(rhs);
		return *this;
	}
};

template <size_t Align, typename Original, typename Mapped>
struct scaffold_lookup {
	using type = scaffold_structural <Align, Mapped>;
};

template <size_t Align, typename Original, typename Mapped>
requires (std::is_fundamental_v <Mapped>)
struct scaffold_lookup <Align, Original, Mapped> {
	using type = scaffold_fundamental <Align, Mapped>;
};

// This will allow us to represent nested things as sequence <A, B, sequence <C, D>, ...>
template <size_t Align, aggregate T, typename ... Ts>
struct scaffold_lookup <Align, T, sequence <Ts...>> {
	using type = T::template scaffold <Align, Ts...>;
};

// TODO: statically sized arrays,
// TODO: unsized arrays...

#define GEN_SCAFFOLD_FIELDS(T, field)					\
	using pad_##field = Ts...[__COUNTER__ - counter_base - 1];	\
	scaffold_lookup <						\
		pad_##field::value,					\
		decltype(T::field),					\
		typename pad_##field::type				\
	> ::type field;

// TODO: require type_and_alignment inputs
#define DEFINE_SCAFFOLD(...)						\
	template <size_t Align, typename ... Ts>			\
	requires (sizeof...(Ts) == reflection::field_count)		\
	struct alignas(Align) scaffold {				\
		static constexpr size_t counter_base = __COUNTER__;	\
		MAP(GEN_SCAFFOLD_FIELDS, This, __VA_ARGS__)		\
	};
	// TODO: using representation = sequence <...>;
