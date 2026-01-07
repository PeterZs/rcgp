#pragma once

#include <type_traits>

#include "../dsl/jems.hpp"
#include "reflection.hpp"
#include "static_string.hpp"

// Fallback with error reported
template <typename T>
void inject_reference(T &value, Reference ref)
{
	static_assert(false, ($ss("failed to inject reference into item of type ") + $ss_type(T)).view());
}

// DSL handles
template <typename T>
requires std::is_base_of_v <jems::handle, T>
void inject_reference(T &value, Reference ref)
{
	value.override_reference(ref);
}

// User-defined types
template <aggregate T, size_t I>
void inject_one_field_of_reference(T &value, Reference ref)
{
	auto &field = value.template _ugp_field_reference <I> ();
	inject_reference(field, jems::field_access(ref, I));
}

template <aggregate T, size_t ... Is>
void inject_all_fields_of_reference(T &value, Reference ref, std::index_sequence <Is...>)
{
	(inject_one_field_of_reference <T, Is> (value, ref), ...);
}

template <aggregate T>
void inject_reference(T &value, Reference ref)
{
	static constexpr auto N = T::reflection::field_count;
	return inject_all_fields_of_reference(value, ref, std::make_index_sequence <N> ());
}
