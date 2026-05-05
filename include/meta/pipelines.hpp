#pragma once

#include <vector>

#include "../dsl/block.hpp"
#include "../util/tlist.hpp"
#include "index_buffer.hpp"

namespace rcgp {

template <typename GAMAP, typename GRCs>
struct GenericPipeline {
	static constexpr size_t set_count = GAMAP::size;

	vk::Pipeline handle;
	vk::PipelineLayout layout;
	std::array <vk::DescriptorSetLayout, set_count> dsls;
	reference_allocation_map gamap;

	GenericPipeline() = default;
	GenericPipeline(
		const vk::Pipeline &handle_,
		const vk::PipelineLayout &layout_,
		const std::array <vk::DescriptorSetLayout, set_count> &dsls_,
		const reference_allocation_map &gamap_
	) : handle(handle_), layout(layout_), dsls(dsls_), gamap(gamap_) {}
};

template <Topology T, typename AS, typename GAMAP, typename GRCs>
struct RasterizationPipeline : GenericPipeline <GAMAP, GRCs> {
	using GenericPipeline <GAMAP, GRCs> ::GenericPipeline;
};

template <typename GAMAP, typename GRCs>
struct ComputePipeline : GenericPipeline <GAMAP, GRCs> {
	using GenericPipeline <GAMAP, GRCs> ::GenericPipeline;
};

template <typename GAMAP, typename GRCs>
struct MeshShadingPipeline : GenericPipeline <GAMAP, GRCs> {
	using GenericPipeline <GAMAP, GRCs> ::GenericPipeline;
};

template <size_t MissCount, size_t ChitCount, typename GAMAP, typename GRCs>
struct RayTracingPipeline : GenericPipeline <GAMAP, GRCs> {
	static constexpr size_t miss_count = MissCount;
	static constexpr size_t chit_count = ChitCount;

	uint32_t trace_group_count = 0;
	std::vector <uint32_t> chit_to_group;

	RayTracingPipeline() = default;
	RayTracingPipeline(
		const vk::Pipeline &handle_,
		const vk::PipelineLayout &layout_,
		const std::array <vk::DescriptorSetLayout, GenericPipeline <GAMAP, GRCs>::set_count> &dsls_,
		const reference_allocation_map &gamap_,
		uint32_t trace_group_count_,
		std::vector <uint32_t> chit_to_group_
	) : GenericPipeline <GAMAP, GRCs> (handle_, layout_, dsls_, gamap_),
	    trace_group_count(trace_group_count_),
	    chit_to_group(std::move(chit_to_group_)) {}
};

TYPE_TRAIT(is_pipeline);
	template <Topology T, typename AS, typename GAMAP, typename GRCs>
	TYPE_TRAIT_INCLUDES(is_pipeline, RasterizationPipeline <T, AS, GAMAP, GRCs>);

	template <typename GA, typename GR>
	TYPE_TRAIT_INCLUDES(is_pipeline, ComputePipeline <GA, GR>);

	template <typename GA, typename GR>
	TYPE_TRAIT_INCLUDES(is_pipeline, MeshShadingPipeline <GA, GR>);

	template <size_t MC, size_t CC, typename GA, typename GR>
	TYPE_TRAIT_INCLUDES(is_pipeline, RayTracingPipeline <MC, CC, GA, GR>);

} // namespace rcgp
