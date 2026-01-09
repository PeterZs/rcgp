#pragma once

#include "../util/cti.hpp"
#include "reflection.hpp"

template <auto &ref>
using reference_base_t = std::decay_t <decltype(ref)>;

// TODO: should enforce that base is a resource type
template <auto &ref>
struct reference : reference_base_t <ref> {
	static constexpr auto &handle = ref;

	using base = reference_base_t <ref>;
	static_assert(has_reflection <base> ());
	using reflection = reference_reflection <ref, typename base::reflection>;
};

TYPE_TRAIT(is_reference);
	template <auto &ref>
	TYPE_TRAIT_INCLUDES(is_reference, reference <ref>);

#define $ref(name)		reference <name> name
#define $tref(name, ...)	reference <name <__VA_ARGS__>> name
