#pragma once

#include <functional>

#include "msv/static_string.hpp"
#include "msv/stage_intrinsics.hpp"
#include "msv/reflection.hpp"
#include "msv/type_string_extensions.hpp"
#include "msv/this_injection.hpp"
#include "msv/type_hash.hpp"
#include "msv/function_return_injection.hpp"
#include "msv/reference.hpp"
#include "msv/stage.hpp"

#include "rhi/session.hpp"
#include "rhi/device.hpp"

#include "dsl/tracer.hpp"

// TODO: UGP namespace

// TODO: ray payloads as resources, not attributes... (ref checks should be easier)

// TODO: layout for parameter blocks is a wrapper layout on the inner type...
// TODO: layout engine type operator...
// TODO: native types have a singleton layout...

// template <auto &resource>
// struct resource_instance {
// 	using value_type = std::remove_reference_t <decltype(resource)>;
//
// 	value_type *operator->() {
// 		// TODO: need to return a proxy handle?
// 		// every jit variable/taggable is either actual code
// 		// or a reference to a slice of staging memory?
// 		// actual GPU memory isnt known until we interface with the pipeline
// 		return new value_type();
// 	}
// };

// TODO: need to inject into the layout...
// deal with this at the same level as before (during jit scoping)

// TODO: aggregate_reflection_t flattening...

// TODO: filtration for function reflection:
// 1. linearize non-parameter block arguments (e.g. vertex attribute packets) in an order preserving way
// 2. hoist out parameter block resources

// template <typename R, typename ... Args>
// auto function_reflection_generator() -> function_reflection <
// 	typename reflection_expander <R> ::type,
// 	typename reflection_expander <std::decay_t <Args>> ::type ...
// >;

// TODO: needs a layout...
template <typename T>
struct ParameterBlock : public T {
	// TODO: lock the members until its used in resource_reference_t
	using reflection = parameter_block_reflection <
		typename reflection_expander <T> ::type
	>;
};

template <typename T>
struct RayPayload : T {};

template <typename R>
struct _return_operator {};

template <typename R, typename U>
void operator<<(_return_operator <R>, const U &value)
{
	static_assert(std::is_convertible_v <U, R>);
	// _return(value);
}

// TODO: pass name explicitly in the decl case, otherwise generate unique ID
template <Stage S, typename Result>
struct _def_operator {};

template <Stage S, typename R, typename ... Args>
struct signature {
	using args = std::tuple <std::decay_t <Args> ...>;
	using returns = R;
	using type = stage <S, R, Args...>;
};

template <Stage S, typename Rt, typename R, typename ... Args>
constexpr auto new_signature(std::function <R (Args...)>) -> signature <S, Rt, Args...>;

// TODO: custom vertex assembler stage

template <Stage S, typename R, typename F>
auto operator<<(_def_operator <S, R>, F ftn)
{
	using function = decltype(std::function(ftn));
	using signature = decltype(new_signature <S, R> (std::declval <function> ()));

	typename signature::type result;

	// auto &em = Emitter::active;

	// em.push(result);
	{
		typename signature::args args;

		// em.display_assembly();
		
		// TODO: generate plceholders during injection, after first filtration

		// TODO: injection
		std::apply(ftn, args);
	}
	// em.pop();
	
	// if (auto &r = $jit.begin(result)) {
	// }

	return result;
}
