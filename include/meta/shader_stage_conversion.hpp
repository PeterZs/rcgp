#pragma once

#include <vulkan/vulkan.hpp>

#include <glslang/Public/ShaderLang.h>

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

} // namespace rcgp
