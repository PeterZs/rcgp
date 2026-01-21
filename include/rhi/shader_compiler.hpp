#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <glslang/Public/ShaderLang.h>

struct ShaderCompiler {
	std::vector <uint32_t> glsl_to_spirv(const std::string &glsl, const EShLanguage &stage) const;

	struct Options {
		bool debug_info = true;
	};

	static ShaderCompiler from(const Options &info);
};
