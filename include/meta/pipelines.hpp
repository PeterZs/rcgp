#pragma once

#include "../rhi/descriptor_pool.hpp"
#include "descriptor.hpp"
#include "group_allocation.hpp"
#include "index_buffer.hpp"
#include "contract.hpp"

namespace rcgp {

template <auto &ref, auto &... refs, size_t ... Is>
constexpr auto set_index_for(const Tlist <group_allocation_record <refs, Is>...> &)
{
	// TODO: use constexpr_for?
	constexpr auto matches = std::array {
		std::same_as <
			contract <ref>,
			contract <refs>
		>...
	};

	constexpr auto index = first_on(matches);
	if constexpr (index < 0) {
		static_error("contract not in group allocation"_ss);
		return 0;
	} else {
		return Is...[index];
	}
}

template <typename GAMAP, typename GRCs>
struct GenericPipeline {
	static constexpr size_t set_count = GAMAP::size;
	
	vk::Device device;
	vk::Pipeline handle;
	vk::PipelineLayout layout;
	std::array <vk::DescriptorSetLayout, set_count> dsls;

	GenericPipeline() = default;

	GenericPipeline(
		const vk::Device &device_,
		const vk::Pipeline &handle_,
		const vk::PipelineLayout &layout_,
		const std::array <vk::DescriptorSetLayout, set_count> &dsls_
	) : device(device_), handle(handle_), layout(layout_), dsls(dsls_) {}

	template <auto &ref>
	auto new_descriptor(const DescriptorPool &pool) const {
		constexpr auto set = set_index_for <ref> (GAMAP());
		
		auto dset = device.allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(pool)
				.setSetLayouts(dsls[set])
		).front();

		return UnboundDescriptor <ref> (dset, set);
	}
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

// Type trait for pipelines
TYPE_TRAIT(is_pipeline);
	template <Topology T, typename AS, typename GAMAP, typename GRCs>
	TYPE_TRAIT_INCLUDES(is_pipeline, RasterizationPipeline <T, AS, GAMAP, GRCs>);

	template <typename GA, typename GR>
	TYPE_TRAIT_INCLUDES(is_pipeline, ComputePipeline <GA, GR>);

	template <typename GA, typename GR>
	TYPE_TRAIT_INCLUDES(is_pipeline, MeshShadingPipeline <GA, GR>);

} // namespace rcgp
