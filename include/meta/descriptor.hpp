#pragma once

#include "../rhi/device.hpp"
#include "reference_introspection.hpp"
#include "resources.hpp"
#include "expand_reflection.hpp"

template <auto &ref, bool resolved = true>
struct DescriptorFor : vk::DescriptorSet {};

// Descriptor writes between descriptor of a resource and its resource mirror
template <typename T>
struct resource_group_value_from_reflection {};

template <typename T>
struct resource_group_value_from_reflection <resource_group_reflection <T>> {
	using type = T;
};

template <typename Original, typename ... Ts>
struct resource_group_value_from_reflection <aggregate_reflection <Original, Ts...>> {
	using type = Original;
};

template <typename R>
struct reflection_unwrap {
	using type = R;
};

template <auto &ref, typename T>
struct reflection_unwrap <reference_reflection <ref, T>> {
	using type = T;
};

// TODO: this doesnt properly account for nested aggregates...
template <typename T>
struct bindings_from_reflection : std::integral_constant <size_t, 1> {};

template <typename T>
struct bindings_from_reflection <resource_group_reflection <T>>
	: std::integral_constant <size_t, expand_reflection_t <T> ::field_count> {};

template <typename Original, typename ... Ts>
struct bindings_from_reflection <aggregate_reflection <Original, Ts...>>
	: std::integral_constant <size_t, sizeof...(Ts)> {};

union DescriptorInfoUnion {
	vk::DescriptorImageInfo image;
	vk::DescriptorBufferInfo buffer;
};

template <auto &ref, bool resolved>
struct DescriptorWritePair {
	const DescriptorFor <ref, resolved> &descriptor;
	const ResourceTypeFor <ref> &resource;

	using Reference = reference_base_t <ref>;
	// TODO: remove and simplify...
	using ref_reflection = typename reflection_unwrap <typename Reference::reflection> ::type;
	static constexpr size_t bindings = is_resource_group_v <Reference>
		? bindings_from_reflection <ref_reflection> ::value
		: 1;
	std::array <DescriptorInfoUnion, bindings> descriptor_infos;

	void bind(const std::span <vk::WriteDescriptorSet, bindings> &writes) {
		if constexpr (is_resource_group_v <Reference>) {
			using Structure = Reference::value_type;
			static_assert(aggregate <Structure>);

			auto bind_one = [&] <size_t I> () {
				using Resource = Structure::reflection::template field_type <I>;
				auto info = resource.template get <I> ().descriptor_info();
				if constexpr (is_sampler_v <Resource>) {
					descriptor_infos[I].image = info;
					writes[I]
						.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
						.setImageInfo(descriptor_infos[I].image);
				} else if constexpr (is_storage_buffer_v <Resource>) {
					descriptor_infos[I].buffer = info;
					writes[I]
						.setDescriptorType(vk::DescriptorType::eStorageBuffer)
						.setBufferInfo(descriptor_infos[I].buffer);
				} else {
					descriptor_infos[I].buffer = info;
					writes[I]
						.setDescriptorType(vk::DescriptorType::eUniformBuffer)
						.setBufferInfo(descriptor_infos[I].buffer);
				}

				writes[I]
					.setDstSet(descriptor)
					.setDstBinding(I);
			};

			constexpr_for(Is, bindings,
				(bind_one.template operator() <Is> (), ...)
			);
		} else {
			// TODO: its the same thing as above, move to a method...
			if constexpr (is_sampler_v <Reference>) {
				descriptor_infos[0].image = resource.descriptor_info();
				writes[0]
					.setDstSet(descriptor)
					.setDstBinding(0)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setImageInfo(descriptor_infos[0].image);
			} else if constexpr (is_storage_buffer_v <Reference>) {
				descriptor_infos[0].buffer = resource.descriptor_info();
				writes[0]
					.setDstSet(descriptor)
					.setDstBinding(0)
					.setDescriptorType(vk::DescriptorType::eStorageBuffer)
					.setBufferInfo(descriptor_infos[0].buffer);
			} else {
				descriptor_infos[0].buffer = resource.descriptor_info();
				writes[0]
					.setDstSet(descriptor)
					.setDstBinding(0)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setBufferInfo(descriptor_infos[0].buffer);
			}
		}
	}
};

template <auto &ref, bool resolved>
DescriptorWritePair(const DescriptorFor <ref, resolved> &,
	const ResourceTypeFor <ref> &)
	-> DescriptorWritePair <ref, resolved>;

template <auto &...refs, bool ... rs>
[[nodiscard]] auto Device::update_descriptors(DescriptorWritePair <refs, rs> ... dwpairs)
{
	static_assert(sizeof...(dwpairs) > 0);

	static constexpr size_t writes_count = (decltype(dwpairs)::bindings + ...);

	std::array <vk::WriteDescriptorSet, writes_count> writes;

	auto progress = 0;
	auto bind = [&](auto &dwpair) {
		using pair_t = std::remove_reference_t <decltype(dwpair)>;
		constexpr size_t size = pair_t::bindings;
		auto span = std::span <vk::WriteDescriptorSet, size> {
			&writes[progress], size
		};
		dwpair.bind(span);
		progress += size;
	};

	(bind(dwpairs), ...);

	logical.updateDescriptorSets(writes, nullptr);

	if constexpr (sizeof...(dwpairs) == 1) {
		return DescriptorFor <refs...[0], true>
			((dwpairs...[0]).descriptor);
	} else {
		return std::make_tuple(DescriptorFor <refs, true> (dwpairs.descriptor)...);
	}
}
