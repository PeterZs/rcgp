#pragma once

#include "../dsl/jems.hpp"
#include "../util/tlist.hpp"
#include "implicit_context.hpp"
#include "reconstruct_type.hpp"

constexpr vk::ShaderStageFlags stage_to_flag(ShaderStage S)
{
	switch (S) {
	case ShaderStage::eVertex: return vk::ShaderStageFlagBits::eVertex;
	case ShaderStage::eFragment: return vk::ShaderStageFlagBits::eFragment;
	case ShaderStage::eCompute: return vk::ShaderStageFlagBits::eCompute;
	case ShaderStage::eTask: return vk::ShaderStageFlagBits::eTaskEXT;
	case ShaderStage::eMesh: return vk::ShaderStageFlagBits::eMeshEXT;
	default:
		return vk::ShaderStageFlagBits::eAll;
	}
}

template <ShaderStage ... Ss>
consteval vk::ShaderStageFlags stage_flags_of()
{
	return (stage_to_flag(Ss) | ... | vk::ShaderStageFlags());
}

// Entrypoint stages
template <ShaderStage S, typename R, typename ... Args>
struct shader_stage : SharedBlockReference {
	shader_stage(const SharedBlockReference &sbr)
		: SharedBlockReference(sbr) {}

	static shader_stage alloc() {
		return std::make_shared <Block> ();
	}
};

// Subroutine stages
template <typename T>
jems::handle coerce_to_handle(const T &value)
{
	static_assert(false, (
		$ss("unable to coerce value of type ")
		+ $ss_type(T)
		+ $ss(" into a jems::handle")
	).view());
}

template <typename T>
requires std::is_base_of_v <jems::handle, T>
auto coerce_to_handle(const T &value)
{
	return value;
}

template <aggregate T>
auto coerce_to_handle(const T &value)
{
	return constexpr_for(Is, T::reflection::field_count,
		return jems::construct(
			reconstruct_type <T> (),
			coerce_to_handle(value.template
				_rcgp_get <Is> ()
			)...
		)
	);
}

template <typename R, typename ... Args>
struct invocable : SharedBlockReference {
	invocable(const SharedBlockReference &sbr)
		: SharedBlockReference(sbr) {}
	
	// TODO: should preserve ref-ness and translate
	// it in the backend IR to generate as in/out/inout for GLSL
	auto operator()(Args ... args) {
		auto inv = jems::invocation(*this, coerce_to_handle(args)...);

		if constexpr (not std::is_same_v <R, void>) {
			R result;
			inject_reference(result, inv);
			return result;
		}
	}
};

// Filter through real parameters for a subroutine
template <typename ... Ts, typename C, typename ... Us>
auto filter_real_parameters(Tlist <Ts...> a, Tlist <C, Us...> processing)
{
	auto next = Tlist <Us...> {};
	if constexpr (is_implicit_context_v <C> or is_reference_v <C>) {
		return filter_real_parameters(a, next);
	} else {
		auto b = Tlist <Ts..., C> {};
		return filter_real_parameters(b, next);
	}
}

template <typename ... Ts>
auto filter_real_parameters(Tlist <Ts...> a, Tlist <> processing)
{
	return a;
}

template <typename R, typename ... Args>
auto filtered_invocable() -> decltype(filter_real_parameters(
	Tlist <R> {},
	Tlist <Args...> {}
)) ::template invoke <invocable>;

template <typename ... Args>
using filtered_invocable_t = decltype(filtered_invocable <Args...> ());

template <typename R, typename ... Args>
struct shader_stage <ShaderStage::eSubroutine, R, Args...> : filtered_invocable_t <R, Args...> {
	using filter_result = decltype(filter_real_parameters(
		Tlist <R> {},
		Tlist <Args...> {}
	));

	shader_stage(const SharedBlockReference &sbr)
		: filtered_invocable_t <R, Args...> (sbr) {}

	static shader_stage alloc() {
		return std::make_shared <Block> ();
	}
};

// TODO: eventually with $context(ftn)(...)
// it will return the invocable instead of ftn being an
// invocable from the start. This will force users to go through $context.
// The exception should be subroutines which dont take any
// resource references/have no context should just be direct though
