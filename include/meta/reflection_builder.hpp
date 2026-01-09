#pragma once

#include "field_name_injection.hpp"
#include "macro_hell.hpp"
#include "reflection.hpp"
#include "scaffold.hpp"
#include "this_injection.hpp"

// Building the reflection for aggregates
#define GEN_AGGREGATE_FIELD(T, field)	\
	, decltype(field)

#define DEFINE_REFLECTION(...)						\
	using reflection = aggregate_reflection <This			\
		MAP(GEN_AGGREGATE_FIELD, /* NA */ , __VA_ARGS__)	\
	>;

// Mapping each field to a unique (local) index
#define GEN_AGGREGATE_FIELD_COUNTER(T, field)	\
	std::integral_constant <size_t, __COUNTER__ - counter_base - 1> field;
	
#define DEFINE_FIELDS(...)							\
	struct field_counter {							\
		static constexpr size_t counter_base = __COUNTER__;		\
		MAP(GEN_AGGREGATE_FIELD_COUNTER, This, __VA_ARGS__)		\
	};									\
	MAP(FIELD_NAME_INJECTION, This, __VA_ARGS__)				\

// Generating a tuple-like get method
#define GEN_AGGREGATE_FIELD_REFERENCE(T, field)	\
	else if constexpr (D == ((__COUNTER__ - 1) - counter_base)) { return field; }

#define DEFINE_FIELD_REFERENCE(R, ...)						\
	template <size_t D>							\
	auto &_rcgp_get() R {							\
		static constexpr size_t counter_base = __COUNTER__;		\
		if constexpr (false) {}						\
		MAP(GEN_AGGREGATE_FIELD_REFERENCE, /* NA */, __VA_ARGS__)	\
		else {								\
			static_assert(false,					\
				($ss("out of bounds field reference access of ")\
				+ $ss_type(This)).view());			\
			return _rcgp_get_fallback;				\
		}								\
	}									\

// Generating the override reference method
#define GEN_OVERRIDE_FIELD_REFERENCE(T, field)	\
	field.override_reference(jems::field_access(ref, __COUNTER__ - counter_base - 1));

#define DEFINE_OVERRIDE_REFERENCE(...)						\
	void override_reference(const Reference &ref) {				\
		static constexpr size_t counter_base = __COUNTER__;		\
		MAP(GEN_OVERRIDE_FIELD_REFERENCE, /* ... */, __VA_ARGS__)	\
	}									\

// Manual marker to indicate membership in the reflection system
// TODO: should just be RCGP_SUPPORTED and supported instead of reflected
#define DEFINE_REFLECTION_STAMP() static constexpr bool _rcgp_has_reflection = true

// Full reflection information for aggregates
#define $reflection(...)					\
	DEFINE_THIS();						\
	DEFINE_REFLECTION_STAMP();				\
								\
	DEFINE_REFLECTION(__VA_ARGS__);				\
	DEFINE_FIELDS(__VA_ARGS__);				\
	DEFINE_SCAFFOLD(__VA_ARGS__);				\
								\
	static inline std::nullptr_t _rcgp_get_fallback {};	\
	DEFINE_FIELD_REFERENCE(/*&*/, __VA_ARGS__);		\
	DEFINE_FIELD_REFERENCE(const, __VA_ARGS__);		\
								\
	DEFINE_OVERRIDE_REFERENCE(__VA_ARGS__);
