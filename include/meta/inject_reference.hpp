#pragma once

#include <type_traits>

#include "../dsl/jems.hpp"
#include "reflection.hpp"
#include "resources.hpp"
#include "static_string.hpp"

// DSL handles
template <typename T>
requires std::is_base_of_v <jems::handle, T>
void inject_reference(T &value, Reference ref)
{
	value.override_reference(ref);
}

// User-defined types
template <aggregate T>
void inject_reference(T &value, Reference ref)
{
	cti_constexpr_for(Is, T::reflection::field_count,
		(inject_reference(
			value.template _ugp_field_reference <Is> (),
			jems::field_access(ref, Is)
		), ...)
	);
}

// TODO: this applies to all buffer handles...
template <typename T, template <typename> typename L>
void inject_reference(PushConstant <T, L> &value, Reference ref)
{
	return inject_reference(static_cast <T &> (value), ref);
}

template <typename T, template <typename> typename L>
void inject_reference(StorageBuffer <T, L> &value, Reference ref)
{
	return inject_reference(static_cast <T &> (value), ref);
}

// Fallback with error reported
template <typename T>
void inject_reference(T &value, Reference ref)
{
	static_assert(false, ($ss("failed to inject reference into item of type ") + $ss_type(T)).view());
}
