#pragma once

#include <vulkan/vulkan.hpp>

#include <glslang/Public/ShaderLang.h>

#include "../dsl/jems.hpp"
#include "../util/tlist.hpp"
#include "implicit_context.hpp"
#include "coerce_to_handle.hpp"

namespace rcgp {

//  TODO: move these to some other file
consteval vk::ShaderStageFlagBits stage_to_flag(ShaderStage S)
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

consteval EShLanguage stage_to_esh(ShaderStage S)
{
	switch (S) {
	case ShaderStage::eVertex: return EShLangVertex;
	case ShaderStage::eFragment: return EShLangFragment;
	case ShaderStage::eCompute: return EShLangCompute;
	case ShaderStage::eTask: return EShLangTask;
	case ShaderStage::eMesh: return EShLangMesh;
	default:
		return EShLangCompute;
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
	static constexpr auto stage = S;
	
	using icontext = icontext_from_args_t <Args...>;

	shader_stage(const SharedBlockReference &sbr)
		: SharedBlockReference(sbr) {}

	static shader_stage alloc() {
		return std::make_shared <Block> ();
	}
};

// Subroutine stages
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
	if constexpr (is_implicit_context_v <C>
			or is_reference_v <C>
			or std::is_same_v <C, std::nullptr_t>) {
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

template <auto &... refs, typename R, typename ... Args>
auto context_unlock(implicit_context <refs...>, const shader_stage <ShaderStage::eSubroutine, R, Args...> &stage)
{
	// TODO: grab the implicit context and check that refs contains everything
	return filtered_invocable_t <R, Args...> (stage);
}

// TODO: The exception should be subroutines which dont take any
// resource references/have no context should just be direct though
#define $use(subroutine) context_unlock(_ctx, subroutine)

} // namespace rcgp
