#pragma once

#include <vulkan/vulkan.hpp>

#include "../dsl/enumerations.hpp"

namespace rcgp {

consteval vk::ShaderStageFlagBits stage_to_flag(ShaderStage S)
{
	switch (S) {
	case ShaderStage::eVertex: return vk::ShaderStageFlagBits::eVertex;
	case ShaderStage::eFragment: return vk::ShaderStageFlagBits::eFragment;
	case ShaderStage::eCompute: return vk::ShaderStageFlagBits::eCompute;
	case ShaderStage::eTask: return vk::ShaderStageFlagBits::eTaskEXT;
	case ShaderStage::eMesh: return vk::ShaderStageFlagBits::eMeshEXT;
	case ShaderStage::eRayGeneration: return vk::ShaderStageFlagBits::eRaygenKHR;
	case ShaderStage::eClosestHit: return vk::ShaderStageFlagBits::eClosestHitKHR;
	case ShaderStage::eMiss: return vk::ShaderStageFlagBits::eMissKHR;
	default:
		return vk::ShaderStageFlagBits::eAll;
	}
}

template <ShaderStage ... Ss>
consteval vk::ShaderStageFlags stage_flags_of()
{
	return (stage_to_flag(Ss) | ... | vk::ShaderStageFlags());
}

} // namespace rcgp
