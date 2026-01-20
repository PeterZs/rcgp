#pragma once

#include <array>

#include "../../rhi/descriptor_pool.hpp"
#include "../../util/cti.hpp"
#include "../descriptor.hpp"
#include "../group_allocation.hpp"
#include "../static_string.hpp"

// TODO: this is the same...
template <auto &ref, auto &... refs, size_t ... Is>
constexpr auto set_index_for_compute(const Tlist <group_allocation_record <refs, Is>...> &)
{
	constexpr auto matches = std::array {
		std::same_as <
			reference <ref>,
			reference <refs>
		>...
	};

	constexpr auto index = first_on(matches);
	if constexpr (index < 0) {
		static_error("reference not in group allocation"_ss);
		return 0;
	} else {
		return Is...[index];
	}
}

template <typename GAMAP, typename GRCs>
struct ComputePipeline {
	static constexpr size_t Sets = GAMAP::size;

	vk::Device device;
	vk::Pipeline handle;
	vk::PipelineLayout layout;
	std::array <vk::DescriptorSetLayout, Sets> dsls;

	using global_resources = GRCs;

	template <auto &ref>
	auto new_descriptor(const DescriptorPool &pool) const {
		constexpr auto set = set_index_for_compute <ref> (GAMAP());
		
		auto dset = device.allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(pool)
				.setSetLayouts(dsls[set])
		).front();

		return DescriptorFor <ref, false> (dset, set);
	}
};
