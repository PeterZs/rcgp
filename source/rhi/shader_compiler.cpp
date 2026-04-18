#include <cstring>

#include <fmt/printf.h>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "dsl/block_canonical.hpp"
#include "dsl/enumerations.hpp"
#include "dsl/generators.hpp"
#include "rhi/shader_cache.hpp"
#include "rhi/shader_compiler.hpp"

namespace rcgp {

EShLanguage esh_stage(ShaderStage stage)
{
	switch (stage) {
	case ShaderStage::eVertex: return EShLangVertex;
	case ShaderStage::eFragment: return EShLangFragment;
	case ShaderStage::eCompute: return EShLangCompute;
	case ShaderStage::eTask: return EShLangTask;
	case ShaderStage::eMesh: return EShLangMesh;
	case ShaderStage::eRayGeneration: return EShLangRayGen;
	case ShaderStage::eClosestHit: return EShLangClosestHit;
	case ShaderStage::eMiss: return EShLangMiss;
	default:
		return EShLangCompute;
	}
}

auto ShaderCompiler::glsl_to_spirv(const std::string &glsl, ShaderStage stage) const
	-> std::vector <uint32_t>
{
	auto defaults = GetDefaultResources();
	auto glslang_stage = esh_stage(stage);

	const char *cstr[] = { glsl.c_str() };

	glslang::SpvOptions options;
	options.generateDebugInfo = debug_info;
	options.disableOptimizer = false;

	glslang::TShader shader(glslang_stage);

	shader.setStrings(cstr, 1);
	shader.setEnvTarget(
		glslang::EShTargetLanguage::EShTargetSpv,
		glslang::EShTargetLanguageVersion::EShTargetSpv_1_6
	);

	EShMessages messages = EShMessages {
		EShMsgDefault
		| EShMsgSpvRules
		| EShMsgVulkanRules
		| EShMsgDebugInfo
	};

	if (!shader.parse(defaults, version, false, messages)) {
		std::string log = shader.getInfoLog();
		fmt::println("failed to compile to SPIRV:\n{}", log);
		return {};
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(messages)) {
		std::string log = program.getInfoLog();
		fmt::println("failed to link SPIRV code:\n{}", log);
		return {};
	}

	std::vector <uint32_t> spirv;
	glslang::GlslangToSpv(*program.getIntermediate(glslang_stage), spirv, &options);
	return spirv;
}

auto ShaderCompiler::compile(const SharedBlockReference &block, ShaderStage stage) const
	-> CompileArtifacts
{
	CompileArtifacts out;

	if (!cache.config.enabled) {
		out.glsl = generate_glsl(block);
		out.spirv = glsl_to_spirv(out.glsl, stage);
		return out;
	}

	auto canonical = canonicalize_block(block);

	CacheFileHeader hdr {};
	std::memcpy(hdr.magic, "RSPV", 4);
	hdr.cache_format_ver = CACHE_FORMAT_VERSION;
	hdr.glslang_ver_hash = glslang_version_token();
	hdr.payload_kind = 1;
	hdr.stage = uint32_t(stage);
	hdr.debug_info = debug_info ? 1u : 0u;
	hdr.glsl_version = version;

	if (auto hit = cache.load_spirv(canonical, hdr)) {
		out.spirv = std::move(*hit);
		out.spirv_cache_hit = true;
		return out;
	}

	if (auto hit = cache.load_glsl(canonical)) {
		out.glsl = std::move(*hit);
		out.glsl_cache_hit = true;
	} else {
		out.glsl = generate_glsl(block);
		cache.store_glsl(canonical, out.glsl);
	}

	out.spirv = glsl_to_spirv(out.glsl, stage);
	if (!out.spirv.empty())
		cache.store_spirv(canonical, out.spirv, hdr);

	return out;
}

} // namespace rcgp
