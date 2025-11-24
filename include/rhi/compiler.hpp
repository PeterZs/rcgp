#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <glslang/Public/ShaderLang.h>

#include "device.hpp"

struct Compiler {
	const vk::Device &device;

	struct Info {
		// TODO: pipeline state?
	};

	std::vector <uint32_t> glsl_to_spirv(const std::string &glsl, const EShLanguage &stage) const;
	
	vk::ShaderModule spirv_to_shader_module(const std::vector <uint32_t> &spirv) const;

	static Compiler from(const Device &device, const Info &info);
};
