#pragma once

#include "layout/all.hpp"
#include "reflection.hpp"
#include "expand_reflection.hpp"

// Data mirrors
template <reflected T, template <typename> typename L = layouts::std430>
struct data_mirror {
	using layout = L <expand_reflection_t <T>>;
	using hint = layout::hint;
	using type = scaffold_lookup <hint, T, true> ::type;
};

#define $mirror(T, ...) data_mirror <T __VA_OPT__(,) __VA_ARGS__> ::type
