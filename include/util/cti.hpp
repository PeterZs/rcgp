#pragma once

// Compile-time index (constant, list, sequence)
template <size_t I>
using cti = std::integral_constant <size_t, I>;

template <size_t ... Is>
using cti_list = std::index_sequence <Is...>;

template <size_t N>
constexpr auto cti_seq = std::make_index_sequence <N> ();

#define cti_constexpr_for(name, N, ...)			\
	[&] <size_t ... name> (cti_list <name...>) {	\
		__VA_ARGS__;				\
	} (cti_seq <N>)

// Casting
template <typename T>
auto &&Tas(auto &&x)
{
	return static_cast <T> (x);
}
