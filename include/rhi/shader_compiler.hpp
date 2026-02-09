#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace rcgp {

enum class ShaderStage;

struct ShaderCompiler {
	bool debug_info = true;
	uint32_t version = 460;

	std::vector <uint32_t> glsl_to_spirv(const std::string &glsl, ShaderStage stage) const;
};

} // namespace rcgp
