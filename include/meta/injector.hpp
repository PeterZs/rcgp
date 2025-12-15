#pragma once

#include <type_traits>

#include "../dsl/jems.hpp"
#include "reference.hpp"
#include "reflection.hpp"
#include "static_string.hpp"
#include "static_access_chain.hpp"

template <typename T, size_t ... Is>
struct field_trace;

// Injecting with a given reference
template <typename T>
struct injector {
	static void main(T &, Reference) {
		static_assert(false, ($ss("failed to inject reference into item of type ") + $ss_type(T)).view());
	}
};

template <typename T>
requires std::is_base_of_v <jems::handle, T>
struct injector <T> {
	static void main(T &item, Reference ref) {
		item.override_reference(ref);
	}
};

// Injecting a reference into a resource at a specific access chain
template <typename T, size_t ... Is>
struct injector <field_trace <T, Is...>> {
	template <auto &rsrc>
	static void main(reference <rsrc> &value, Reference ref) {
		static_access_chain <Is...> (value).override_reference(ref);
	}
};

// Helpers for aggregate field injection kept out-of-class to reduce template bloat
template <auto &rsrc, size_t I>
void inject_aggregate_field(reference <rsrc> &value, Reference ref)
{
	auto &field = value.template _ugp_field_reference <I> ();
	auto access = jems::field_access(ref, I);
	
	using T = std::remove_reference_t <decltype(field)>;
	injector <T> ::main(field, access);
}

template <auto &rsrc, size_t ... Is>
void inject_aggregate_fields(reference <rsrc> &value, Reference ref, std::index_sequence <Is...>)
{
	(inject_aggregate_field <rsrc, Is> (value, ref), ...);
}

// Inject into reflected aggregates by walking their fields in declaration order
template <aggregate T>
struct injector <T> {
	template <auto &rsrc>
	static void main(reference <rsrc> &value, Reference ref) {
		static constexpr auto field_count = T::reflection::field_count;
		inject_aggregate_fields <rsrc> (
			value, ref,
			std::make_index_sequence <field_count> ()
		);
	}
};
