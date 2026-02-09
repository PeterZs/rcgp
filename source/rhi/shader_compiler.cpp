#include <fmt/printf.h>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "dsl/enumerations.hpp"
#include "rhi/shader_compiler.hpp"
#include "util/timer.hpp"

namespace rcgp {

namespace {

EShLanguage to_glslang_stage(ShaderStage stage)
{
	switch (stage) {
	case ShaderStage::eVertex: return EShLangVertex;
	case ShaderStage::eFragment: return EShLangFragment;
	case ShaderStage::eCompute: return EShLangCompute;
	case ShaderStage::eTask: return EShLangTask;
	case ShaderStage::eMesh: return EShLangMesh;
	default:
		return EShLangCompute;
	}
}

} // namespace

std::vector <uint32_t> ShaderCompiler::glsl_to_spirv(const std::string &glsl, ShaderStage stage) const
{
	auto defaults = GetDefaultResources();
	auto glslang_stage = to_glslang_stage(stage);

	TSCOPE("compile glsl to spirv");

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

} // namespace rcgp
