#pragma once

#include "../../dsl/generators.hpp"
#include "../../dsl/instructions.hpp"
#include "../../rhi/shader_compiler.hpp"
#include "../../util/logging.hpp"
#include "../attribute_description.hpp"
#include "../binding_description.hpp"
#include "../collect_gvrs.hpp"
#include "../collect_streams.hpp"
#include "../group_allocation.hpp"
#include "../implicit_context.hpp"
#include "../pipeline/rasterization.hpp"
#include "../shader_stage.hpp"

// TODO: group allocation record should be a proper type...
// template <...> struct group_allocation { get_set_for <ref> = ... };
template <auto &ref, auto &... refs, size_t ... Is>
constexpr auto group_allocation_set_for(const std::tuple <group_allocation_record <refs, Is>...> &)
{
	constexpr auto matches = std::array {
		std::same_as <
			reference <ref>,
			reference <refs>
		>...
	};

	constexpr auto index = first_on(matches);
	if constexpr (index < 0) {
		static_assert(false, "reference not in group allocation");
		return 0;
	} else {
		return Is...[index];
	}
}

template <auto &... refs, size_t ... Is>
auto sequence_to_group_allocation_impl(
	const sequence <reference <refs>...> &,
	const std::index_sequence <Is...> &
)
{
	return std::make_tuple(group_allocation_record <refs, Is> ()...);
}

template <typename ... Ts>
auto sequence_to_group_allocation(const sequence <Ts...> &)
{
	return sequence_to_group_allocation_impl(
		sequence <typename Ts::type...> ::reify(),
		std::make_index_sequence <sizeof...(Ts)> ()
	);
}

// Filter helpers for separating descriptor-backed resources from push constants
template <typename Seq>
struct descriptor_resource_filter;

template <>
struct descriptor_resource_filter <sequence <>> {
	using type = sequence <>;
};

template <auto &ref, ShaderStage ...Ss, typename ...Rest>
struct descriptor_resource_filter <sequence <stage_wrapper <reference <ref>, Ss...>, Rest...>> {
	using base = typename reference <ref> ::base;
	using tail = typename descriptor_resource_filter <sequence <Rest...>> ::type;
	using type = std::conditional_t <
		is_push_constant_v <base>,
		tail,
		typename tail::template push_front_t <stage_wrapper <reference <ref>, Ss...>>
	>;
};

template <typename Seq>
using descriptor_resources_t = typename descriptor_resource_filter <Seq> ::type;

template <typename Seq>
struct push_constant_filter;

template <>
struct push_constant_filter <sequence <>> {
	using type = sequence <>;
};

template <auto &ref, ShaderStage ...Ss, typename ...Rest>
struct push_constant_filter <sequence <stage_wrapper <reference <ref>, Ss...>, Rest...>> {
	using base = typename reference <ref> ::base;
	using tail = typename push_constant_filter <sequence <Rest...>> ::type;
	using type = std::conditional_t <
		is_push_constant_v <base>,
		typename tail::template push_front_t <stage_wrapper <reference <ref>, Ss...>>,
		tail
	>;
};

template <typename Seq>
using push_constant_resources_t = typename push_constant_filter <Seq> ::type;

// TODO: need to manage names...
// TODO: also need to clean this stuff up...
template <auto &ref, ShaderStage ... Ss>
auto reference_to_descriptor_set_layout(const Device &device, const stage_wrapper <reference <ref>, Ss...> &)
{
	using base = typename reference <ref> ::base;
	using ref_reflection = typename base::reflection;
	auto stage_flags = (stage_to_flag(Ss) | ...);

	if constexpr (is_resource_group_v <base>) {
		using group_value = typename ref_reflection::value_type;
		using group_reflection = expand_reflection_t <group_value>;
		static_assert(is_aggregate_reflection_v <group_reflection>,
			"resource group must wrap an aggregate type");

		constexpr size_t bindings = group_reflection::field_count;
		std::array <vk::DescriptorSetLayoutBinding, bindings> dslbs {};

		auto fill_one = [&] <size_t I> () {
			using field_t = typename group_reflection::template field_type <I>;
			vk::DescriptorType dtype = vk::DescriptorType::eUniformBuffer;
			if constexpr (is_sampler_reflection_v <field_t>) {
				dtype = vk::DescriptorType::eCombinedImageSampler;
			} else if constexpr (is_storage_buffer_reflection_v <field_t>) {
				dtype = vk::DescriptorType::eStorageBuffer;
			}

			dslbs[I] = vk::DescriptorSetLayoutBinding()
				.setBinding(static_cast <uint32_t> (I))
				.setDescriptorCount(1)
				.setDescriptorType(dtype)
				.setStageFlags(stage_flags);
		};

		[&] <size_t ... Is> (std::index_sequence <Is...>) {
			(fill_one.template operator() <Is> (), ...);
		} (std::make_index_sequence <bindings> ());

		auto dsl_info = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(dslbs);

		return device.logical.createDescriptorSetLayout(dsl_info);
	} else {
		// TODO: method for generating DescriptorType for global resources
		vk::DescriptorType dtype = vk::DescriptorType::eUniformBuffer;
		if constexpr (is_sampler_v <base>) {
			dtype = vk::DescriptorType::eCombinedImageSampler;
		} else if constexpr (is_storage_buffer_reflection_v <ref_reflection>) {
			dtype = vk::DescriptorType::eStorageBuffer;
		}

		auto dslbs = std::array {
			vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(dtype)
				.setStageFlags(stage_flags)
		};

		auto dsl_info = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(dslbs);

		return device.logical.createDescriptorSetLayout(dsl_info);
	}
}

template <typename ... Ts>
auto reference_sequence_to_descriptor_set_layouts(const Device &device, const sequence <Ts...> &)
{
	// TODO: separate arrays for dsl and pc ranges
	if constexpr (sizeof...(Ts) == 0) {
		return std::array <vk::DescriptorSetLayout, 0> ();
	} else {
		return std::array {
			reference_to_descriptor_set_layout(device, Ts())...
		};
	}
}

template <auto &ref, ShaderStage ... Ss>
auto reference_to_push_constant_range(const stage_wrapper <reference <ref>, Ss...> &, uint32_t offset)
{
	using info = push_constant_info <stage_wrapper <reference <ref>, Ss...>>;

	auto range = vk::PushConstantRange()
		.setOffset(offset)
		.setSize(info::size)
		.setStageFlags(info::stage_flags);

	return range;
}

template <typename ... Ts>
auto reference_sequence_to_push_constant_ranges(const sequence <Ts...> &)
{
	if constexpr (sizeof...(Ts) == 0) {
		return std::array <vk::PushConstantRange, 0> ();
	} else {
		std::array <vk::PushConstantRange, sizeof...(Ts)> ranges {};
		uint32_t offset = 0;
		size_t index = 0;

		auto add = [&](auto tag) {
			using info = push_constant_info <decltype(tag)>;
			offset = align_up_u32(offset, info::alignment);
			ranges[index++] = reference_to_push_constant_range(tag, offset);
			offset += info::size;
		};

		(add(Ts()), ...);
		return ranges;
	}
}

template <typename ... Ts>
auto reference_sequence_to_push_constant_allocation_map(const sequence <Ts...> &)
{
	push_constant_allocation_map map;
	if constexpr (sizeof...(Ts) > 0) {
		uint32_t offset = 0;
		uint32_t index = 0;

		auto add = [&](auto tag) {
			using info = push_constant_info <decltype(tag)>;
			offset = align_up_u32(offset, info::alignment);
			map.emplace(info::addr, PushConstantAllocation { index++, offset });
			offset += info::size;
		};

		(add(Ts()), ...);
	}
	return map;
}

consteval vk::PrimitiveTopology translate_topology(Topology T)
{
	switch (T) {
	case Topology::eTriangleList: return vk::PrimitiveTopology::eTriangleList;
	case Topology::eTriangleFan: return vk::PrimitiveTopology::eTriangleFan;
	}

	return vk::PrimitiveTopology::eTriangleList;
}

struct RasterizationOptions {
	// TODO: dynamic extent or fixed extent
	const vk::Extent2D &extent;
	bool depth_test;
};

template <auto T>
struct marker {};

template <auto T>
static constexpr auto marker_v = marker <T> {};

template <Topology T, typename ... Subpasses>
struct RasterizationCombinator {
	const marker <T> &topology;
	const Device &device;
	const ShaderCompiler &compiler;
	const RenderPass <Subpasses...> &render_pass;
	RasterizationOptions options;

	template <typename VRet, typename ... As, typename FRet, typename ... Bs>
	auto operator()(shader_stage <ShaderStage::eVertex, VRet, As...> &vertex,
		 	shader_stage <ShaderStage::eFragment, FRet, Bs...> &fragment) const {
		// TODO: check between vshader output and fshader input
		// TODO: store # of attachments required from # of fshader outputs
		using vertex_icontext = find_implicit_context <As...> ::type;
		using fragment_icontext = find_implicit_context <Bs...> ::type;

		// Collect vertex attribute streams
		auto streams0 = sequence <> {};
		auto streams = add_stream_from_implicit_context(streams0, vertex_icontext());

		// Generate vertex input bindings and attributes
		auto vbindings = sequence_to_vertex_bindings(streams);
		auto vattributes = sequence_to_vertex_attributes(streams);

		// Collect global resources
		auto gvrs0 = sequence <> ::reify();
		auto gvrs1 = add_gvr_from_implicit_context <ShaderStage::eVertex> (gvrs0, vertex_icontext());
		auto gvrs = add_gvr_from_implicit_context <ShaderStage::eFragment> (gvrs1, fragment_icontext());

		auto descriptor_gvrs = descriptor_resources_t <decltype(gvrs)> ::reify();
		auto push_constant_gvrs = push_constant_resources_t <decltype(gvrs)> ::reify();

		auto alloc = sequence_to_group_allocation(descriptor_gvrs);
		auto gamap = new_allocation(alloc);
		vertex->apply_group_allocation_map(gamap);
		fragment->apply_group_allocation_map(gamap);

		auto pcmap = reference_sequence_to_push_constant_allocation_map(push_constant_gvrs);
		vertex->apply_push_constant_allocation_map(pcmap);
		fragment->apply_push_constant_allocation_map(pcmap);

		// Compile the shaders
		auto vshader = generate_glsl(vertex);
		info("vertex shader:\n%s", vshader.c_str());

		auto fshader = generate_glsl(fragment);
		info("fragment shader:\n%s", fshader.c_str());
	
		auto vspv = compiler.glsl_to_spirv(vshader, EShLangVertex);
		auto fspv = compiler.glsl_to_spirv(fshader, EShLangFragment);

		auto vmodule = compiler.spirv_to_shader_module(vspv);
		auto fmodule = compiler.spirv_to_shader_module(fspv);

		// Generate the pipeline and descriptor set layouts
		auto dsls = reference_sequence_to_descriptor_set_layouts(device, descriptor_gvrs);
		auto pcrs = reference_sequence_to_push_constant_ranges(push_constant_gvrs);

		auto layout_info = vk::PipelineLayoutCreateInfo().setSetLayouts(dsls);
		if constexpr (push_constant_gvrs.size > 0)
			layout_info.setPushConstantRanges(pcrs);
		auto layout = device.logical.createPipelineLayout(layout_info);

		// TODO: the rest of this should be an RHI method
		// Building the pipeline
		auto shader_stages = std::array {
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setModule(vmodule)
				.setPName("main"),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setModule(fmodule)
				.setPName("main"),
		};

		auto vertex_input = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(vbindings)
			.setVertexAttributeDescriptions(vattributes);

		auto input_assembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(translate_topology(T))
			.setPrimitiveRestartEnable(false);

		auto viewport = vk::Viewport()
			.setX(0.0f)
			.setY(0.0f)
			.setWidth(float(options.extent.width))
			.setHeight(float(options.extent.height))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		auto scissor = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent(options.extent);

		auto viewport_state = vk::PipelineViewportStateCreateInfo()
			.setViewports(viewport)
			.setScissors(scissor);

		auto rasterization = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(false)
			.setLineWidth(1.0f);

		auto multisampling = vk::PipelineMultisampleStateCreateInfo()
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(false)
			.setMinSampleShading(1.0f);

		auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(false)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA
			);

		auto color_blend = vk::PipelineColorBlendStateCreateInfo()
			.setAttachments(color_blend_attachment);

		auto depth_stencil = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(options.depth_test)
			.setDepthWriteEnable(options.depth_test)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(false)
			.setStencilTestEnable(false);

		auto pipeline_info = vk::GraphicsPipelineCreateInfo()
			.setLayout(layout)
			.setPInputAssemblyState(&input_assembly)
			.setPVertexInputState(&vertex_input)
			.setPRasterizationState(&rasterization)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&color_blend)
			.setPDepthStencilState(&depth_stencil)
			.setPViewportState(&viewport_state)
			.setStages(shader_stages)
			.setRenderPass(render_pass)
			.setSubpass(0);

		auto [result, pipeline] = device.logical.createGraphicsPipeline(nullptr, pipeline_info, nullptr);
		assertion(result == vk::Result::eSuccess, "failed to compile pipeline");

		return AnnotatedRasterizationPipeline <
			T,
			decltype(streams),
			decltype(alloc),
			decltype(gvrs),
			dsls.size()
		> {
			.device = device.logical,
			.handle = pipeline,
			.layout = layout,
			.dsls = dsls,
		};
	}
};
