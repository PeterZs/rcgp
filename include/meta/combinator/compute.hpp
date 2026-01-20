#pragma once

#include "../../dsl/generators.hpp"
#include "../../rhi/pipelines.hpp"
#include "../../rhi/shader_compiler.hpp"
#include "../../util/timer.hpp"
#include "../collect_gvrs.hpp"
#include "../combinator/rasterization.hpp"
#include "../group_allocation.hpp"
#include "../implicit_context.hpp"
#include "../pipeline/compute.hpp"
#include "../shader_stage.hpp"

struct ComputeCombinator {
	const Device &device;
	const ShaderCompiler &compiler;

	template <typename Ret, typename ... As>
	auto operator()(shader_stage <ShaderStage::eCompute, Ret, As...> &compute) const {
		TSCOPE("compute combinator");

		using icontext = icontext_from_args_t <As...>;

		// Collect global resources
		auto gvrs0 = Tlist <> {};
		auto gvrs = add_gvr_from_implicit_context <ShaderStage::eCompute> (gvrs0, icontext());

		auto descriptor_gvrs = descriptable_resources_t <decltype(gvrs)> {};
		auto push_constant_gvrs = push_constant_resources_t <decltype(gvrs)> {};

		auto alloc = sequence_to_group_allocation(descriptor_gvrs);
		auto gamap = new_group_allocation_map(alloc);
		compute->apply_group_allocation_map(gamap);

		auto pcmap = reference_sequence_to_push_constant_allocation_map(push_constant_gvrs);
		compute->apply_push_constant_allocation_map(pcmap);

		// Compile the shader
		auto cshader = generate_glsl(compute);
		// info("compute shader:\n%s", cshader.c_str());

		auto cspv = compiler.glsl_to_spirv(cshader, EShLangCompute);
		auto compute_shader_module = device.new_shader_module(cspv);

		// Generate the pipeline and descriptor set layouts
		auto dsls = reference_sequence_to_descriptor_set_layouts(device, descriptor_gvrs);
		auto pcrs = reference_sequence_to_push_constant_ranges(push_constant_gvrs);

		auto layout_info = vk::PipelineLayoutCreateInfo().setSetLayouts(dsls);
		if constexpr (push_constant_gvrs.size > 0)
			layout_info.setPushConstantRanges(pcrs);
		auto layout = device.logical.createPipelineLayout(layout_info);

		auto pipeline = compile_compute_pipeline(
			device,
			compute_shader_module,
			"main",
			layout
		);

		return ComputePipeline <
			decltype(alloc),
			decltype(gvrs)
		> {
			.device = device.logical,
			.handle = pipeline,
			.layout = layout,
			.dsls = dsls,
		};
	}
};
