#pragma once

#include "process_wrappers.hpp"
#include "group_allocation.hpp"

template <typename ... Ts>
auto sequence_to_group_allocation(const Tlist <Ts...> &)
{
	return constexpr_for(Is, sizeof...(Ts),
		return Tlist <group_allocation_record <Ts::reference::handle, Is>...> {}
	);
}

template <typename GVRs, typename ... Blocks>
auto apply_gvrs(const Device &device, const GVRs &, Blocks &... blocks)
{
	using descriptor_gvrs = descriptable_resources_t <GVRs>;
	using push_constant_gvrs = push_constant_resources_t <GVRs>;

	auto alloc = sequence_to_group_allocation(descriptor_gvrs());
	auto gamap = new_group_allocation_map(alloc);
	(blocks->apply_group_allocation_map(gamap), ...);

	auto pcmap = wrappers_to_pcmap(push_constant_gvrs());
	(blocks->apply_push_constant_allocation_map(pcmap), ...);

	auto dsls = wrappers_to_dsls(device, descriptor_gvrs());
	auto pcrs = wrappers_to_pcrs(push_constant_gvrs());

	auto layout_info = vk::PipelineLayoutCreateInfo().setSetLayouts(dsls);
	if constexpr (push_constant_gvrs::size > 0)
		layout_info.setPushConstantRanges(pcrs);

	auto layout = device.logical.createPipelineLayout(layout_info);

	return std::tuple {
		layout,
		dsls,
		alloc,
	};
}
