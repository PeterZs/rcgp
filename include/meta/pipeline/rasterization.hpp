#pragma once

#include <algorithm>

#include "../../rhi/descriptor_pool.hpp"
#include "../../util/cti.hpp"
#include "../collect_gvrs.hpp"
#include "../descriptor.hpp"
#include "../group_allocation.hpp"
#include "../index_buffer.hpp"
#include "../static_string.hpp"

// Compile-time stage flag lookup for a given resource reference within a sequence
// TODO: function...
template <auto &ref, typename Seq>
struct stage_flags_for_seq;

template <auto &ref>
struct stage_flags_for_seq <ref, Tlist <>> {
	static constexpr vk::ShaderStageFlags value = {};
};

template <auto &ref, auto &other, ShaderStage ...Ss, typename ...Rest>
struct stage_flags_for_seq <
	ref,
	Tlist <stage_wrapper <other, Ss...>, Rest...>
> {
	static constexpr vk::ShaderStageFlags value =
		std::same_as <reference <other>, reference <ref>>
			? stage_flags_of <Ss...> ()
			: stage_flags_for_seq <ref, Tlist <Rest...>> ::value;
};

template <auto &ref, typename Seq>
constexpr vk::ShaderStageFlags stage_flags_for_v = stage_flags_for_seq <ref, Seq> ::value;

// TODO: util?
constexpr uint32_t align_up_u32(uint32_t value, uint32_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

// TODO: different header...
template <auto &ref, typename ... Wrappers>
consteval size_t push_constant_offset_for(Tlist <Wrappers...>)
{
	size_t offset = 0;
	bool passed = false;
	auto accum = [&] <typename W> () {
		using R = W::type;
		using T = ResourceTypeFor <W::reference::handle>;

		if constexpr (std::same_as <typename W::reference, reference <ref>>)
			passed = true;
		if (not passed && is_push_constant_v <R>) {
			offset = align_up_u32(offset, alignof(T));
			offset += sizeof(T);
		}
	};
	(accum.template operator() <Wrappers> (), ...);
	return offset;
}

// TODO: different header...
template <auto &ref, auto &... refs, size_t ... Is>
constexpr auto set_index_for(const Tlist <group_allocation_record <refs, Is>...> &)
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

template <Topology T, typename AS, typename GAMAP, typename GRCs>
struct RasterizationPipeline {
	static constexpr size_t Sets = GAMAP::size;

	vk::Device device;
	vk::Pipeline handle;
	vk::PipelineLayout layout;
	std::array <vk::DescriptorSetLayout, Sets> dsls;

	using global_resources = GRCs;
	using streams = AS;

	template <auto &ref>
	auto new_descriptor(const DescriptorPool &pool) const {
		constexpr auto set = set_index_for <ref> (GAMAP());
		
		auto dset = device.allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(pool)
				.setSetLayouts(dsls[set])
		).front();

		return DescriptorFor <ref, false> (dset, set);
	}
};
