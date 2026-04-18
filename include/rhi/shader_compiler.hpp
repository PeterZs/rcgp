#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "../dsl/instruction_nodes.hpp"
#include "shader_cache.hpp"

namespace rcgp {

enum class ShaderStage;

struct CompileArtifacts {
	std::string glsl;
	std::vector <uint32_t> spirv;
	bool glsl_cache_hit = false;
	bool spirv_cache_hit = false;
};

struct ShaderCompiler {
	bool debug_info = true;
	uint32_t version = 460;
	ShaderCache cache;

	auto glsl_to_spirv(const std::string &glsl, ShaderStage stage) const -> std::vector <uint32_t>;
	auto compile(const SharedBlockReference &block, ShaderStage stage) const -> CompileArtifacts;
};

} // namespace rcgp
