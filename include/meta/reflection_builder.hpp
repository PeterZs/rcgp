#pragma once

#include "macro_hell.hpp"
#include "scaffold.hpp"
#include "this_injection.hpp"

template <int X, typename T>
using _counter_field = T;

// Building the reflection for aggregates
#define GEN_AGGREGATE_FIELD(T, field)	, _counter_field <__COUNTER__, decltype(field)>

// TODO: use a tlist
#define DEFINE_REFLECTION(...)								\
	static constexpr size_t _field_counter_start = __COUNTER__;			\
	using reflection = aggregate_reflection <This					\
		MAP(GEN_AGGREGATE_FIELD, /* NA */ , __VA_ARGS__)			\
	>;										\
	static constexpr size_t field_count = (__COUNTER__ - _field_counter_start - 1);

// Generating a tuple-like get method
#define GEN_AGGREGATE_FIELD_REFERENCE(T, field)	\
	else if constexpr (D == (__COUNTER__ - counter_base - 1)) { return field; }

#define DEFINE_FIELD_REFERENCE(R, ...)							\
	template <size_t D>								\
	auto &_rcgp_get() R {								\
		static constexpr size_t counter_base = __COUNTER__;			\
		if constexpr (false) {}							\
		MAP(GEN_AGGREGATE_FIELD_REFERENCE, /* NA */, __VA_ARGS__)		\
		else {									\
			static_error("out of bounds field reference access of "_ss	\
				+ $ss_type(This));					\
			return _rcgp_get_fallback;					\
		}									\
	}										\

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
	DEFINE_SCAFFOLD(__VA_ARGS__);				\
								\
	static inline std::nullptr_t _rcgp_get_fallback {};	\
	DEFINE_FIELD_REFERENCE(/*&*/, __VA_ARGS__);		\
	DEFINE_FIELD_REFERENCE(const, __VA_ARGS__);		\
								\
	DEFINE_OVERRIDE_REFERENCE(__VA_ARGS__);
