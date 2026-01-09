#pragma once

#include "../../dsl/generators.hpp"
#include "../../rhi/pipelines.hpp"
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
	const Tlist <reference <refs>...> &,
	const std::index_sequence <Is...> &
)
{
	return std::make_tuple(group_allocation_record <refs, Is> ()...);
}

template <typename ... Ts>
auto sequence_to_group_allocation(const Tlist <Ts...> &)
{
	return sequence_to_group_allocation_impl(
		Tlist <typename Ts::type...> {},
		std::make_index_sequence <sizeof...(Ts)> ()
	);
}

// Filter helpers for separating descriptor-backed resources from push constants
// TODO: use a function to try and eliminate the insert 'method'
template <typename Seq>
struct descriptor_resource_filter;

template <>
struct descriptor_resource_filter <Tlist <>> {
	using type = Tlist <>;
};

template <auto &ref, ShaderStage ...Ss, typename ...Rest>
struct descriptor_resource_filter <Tlist <stage_wrapper <reference <ref>, Ss...>, Rest...>> {
	using base = typename reference <ref> ::base;
	using tail = typename descriptor_resource_filter <Tlist <Rest...>> ::type;
	using type = std::conditional_t <
		is_push_constant_v <base>,
		tail,
		typename tail::template insert <stage_wrapper <reference <ref>, Ss...>>
	>;
};

template <typename Seq>
using descriptor_resources_t = typename descriptor_resource_filter <Seq> ::type;

template <typename Seq>
struct push_constant_filter;

template <>
struct push_constant_filter <Tlist <>> {
	using type = Tlist <>;
};

template <auto &ref, ShaderStage ...Ss, typename ...Rest>
struct push_constant_filter <Tlist <stage_wrapper <reference <ref>, Ss...>, Rest...>> {
	using base = typename reference <ref> ::base;
	using tail = typename push_constant_filter <Tlist <Rest...>> ::type;
	using type = std::conditional_t <
		is_push_constant_v <base>,
		typename tail::template insert <stage_wrapper <reference <ref>, Ss...>>,
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
auto reference_sequence_to_descriptor_set_layouts(const Device &device, const Tlist <Ts...> &)
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
auto reference_sequence_to_push_constant_ranges(const Tlist <Ts...> &)
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
auto reference_sequence_to_push_constant_allocation_map(const Tlist <Ts...> &)
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
		auto streams0 = Tlist <> {};
		auto streams = add_stream_from_implicit_context(streams0, vertex_icontext());

		// Generate vertex input bindings and attributes
		auto vertex_bindings = sequence_to_vertex_bindings(streams);
		auto vertex_attributes = sequence_to_vertex_attributes(streams);

		// Collect global resources
		auto gvrs0 = Tlist <> {};
		auto gvrs1 = add_gvr_from_implicit_context <ShaderStage::eVertex> (gvrs0, vertex_icontext());
		auto gvrs = add_gvr_from_implicit_context <ShaderStage::eFragment> (gvrs1, fragment_icontext());

		auto descriptor_gvrs = descriptor_resources_t <decltype(gvrs)> {};
		auto push_constant_gvrs = push_constant_resources_t <decltype(gvrs)> {};

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

		auto vertex_shader_module = compiler.spirv_to_shader_module(vspv);
		auto fragment_shader_module = compiler.spirv_to_shader_module(fspv);

		// Generate the pipeline and descriptor set layouts
		auto dsls = reference_sequence_to_descriptor_set_layouts(device, descriptor_gvrs);
		auto pcrs = reference_sequence_to_push_constant_ranges(push_constant_gvrs);

		auto layout_info = vk::PipelineLayoutCreateInfo().setSetLayouts(dsls);
		if constexpr (push_constant_gvrs.size > 0)
			layout_info.setPushConstantRanges(pcrs);
		auto layout = device.logical.createPipelineLayout(layout_info);

		auto pipeline = compile_rasterization_pipeline(
			device,
			render_pass,
			translate_topology(T),
			vertex_shader_module,
			fragment_shader_module,
			layout,
			vertex_bindings,
			vertex_attributes,
			options
		);

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
