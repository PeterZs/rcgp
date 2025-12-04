#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

struct DescriptorPool : vk::DescriptorPool {
	struct Info {
		uint32_t max_sets = 0;
		uint32_t samplers = 0;
		uint32_t combined_image_samplers = 0;
		uint32_t sampled_images = 0;
		uint32_t storage_images = 0;
		uint32_t uniform_texel_buffers = 0;
		uint32_t storage_texel_buffers = 0;
		uint32_t uniform_buffers = 0;
		uint32_t storage_buffers = 0;
		uint32_t uniform_buffers_dynamic = 0;
		uint32_t storage_buffers_dynamic = 0;
		uint32_t input_attachments = 0;
		uint32_t inline_uniform_blocks = 0; // VK_EXT_inline_uniform_block
		uint32_t acceleration_structures = 0; // VK_KHR_acceleration_structure
	};

	static DescriptorPool from(const Device &device, const Info &info)
	{
		std::vector <vk::DescriptorPoolSize> sizes;
		sizes.reserve(14);

		auto push = [&sizes](vk::DescriptorType type, uint32_t count) {
			if (count)
				sizes.emplace_back(type, count);
		};

		push(vk::DescriptorType::eSampler, info.samplers);
		push(vk::DescriptorType::eCombinedImageSampler, info.combined_image_samplers);
		push(vk::DescriptorType::eSampledImage, info.sampled_images);
		push(vk::DescriptorType::eStorageImage, info.storage_images);
		push(vk::DescriptorType::eUniformTexelBuffer, info.uniform_texel_buffers);
		push(vk::DescriptorType::eStorageTexelBuffer, info.storage_texel_buffers);
		push(vk::DescriptorType::eUniformBuffer, info.uniform_buffers);
		push(vk::DescriptorType::eStorageBuffer, info.storage_buffers);
		push(vk::DescriptorType::eUniformBufferDynamic, info.uniform_buffers_dynamic);
		push(vk::DescriptorType::eStorageBufferDynamic, info.storage_buffers_dynamic);
		push(vk::DescriptorType::eInputAttachment, info.input_attachments);
		push(vk::DescriptorType::eInlineUniformBlockEXT, info.inline_uniform_blocks);
		push(vk::DescriptorType::eAccelerationStructureKHR, info.acceleration_structures);

		auto pool_info = vk::DescriptorPoolCreateInfo()
			.setMaxSets(info.max_sets)
			.setPoolSizes(sizes);

		return DescriptorPool(device.logical.createDescriptorPool(pool_info));
	}
};

