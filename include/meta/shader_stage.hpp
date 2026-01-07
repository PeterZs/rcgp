#pragma once

#include <type_traits>

#include "../dsl/tracer.hpp"
#include "../util/sequence.hpp"

enum class ShaderStage {
	eUndefined,

	// Standard stages after compilation
	eVertex,
	eFragment,
	eCompute,
};

constexpr vk::ShaderStageFlags stage_to_flag(ShaderStage S)
{
	switch (S) {
	case ShaderStage::eVertex: return vk::ShaderStageFlagBits::eVertex;
	case ShaderStage::eFragment: return vk::ShaderStageFlagBits::eFragment;
	default:
		return vk::ShaderStageFlagBits::eAll;
	}
}

template <ShaderStage ... Ss>
consteval vk::ShaderStageFlags stage_flags_of()
{
	return (vk::ShaderStageFlags) (stage_to_flag(Ss) | ... | vk::ShaderStageFlags());
}

template <ShaderStage S, typename R, typename ... Args>
struct shader_stage : Block {};
