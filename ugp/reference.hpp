#pragma once

#include <type_traits>

#include "reflection.hpp"
#include "static_string.hpp"

template <auto &resource>
using reference_base = std::remove_reference_t <decltype(resource)>;

template <auto &resource>
struct reference : reference_base <resource> {
	using value_type = reference_base <resource>;

	static_assert(reflection_expander <value_type> ::value,
	       // TODO: string only format style...
	       ($ss_type(value_type) + $ss(
	       " has no valid reflection, perhaps you forgot to"
	       " add it to the registry via $reflection(...)?")).view());

	using reflection = reference_reflection <
		resource,
		typename value_type::reflection
	>;
};

#define $use(name)	reference <name> name

// TODO: for comparisons just use std::same_as...
