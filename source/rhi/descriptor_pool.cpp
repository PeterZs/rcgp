#include "rhi/descriptor_pool.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace rcgp {

DescriptorPool DescriptorPool::from(const Device &device, const Options &info)
{
	std::vector <vk::DescriptorPoolSize> sizes;
	sizes.reserve(14);

	uint64_t total_sets = 0;

	auto push = [&](vk::DescriptorType type, uint32_t count) {
		if (count)
			sizes.emplace_back(type, count);
		total_sets += uint64_t(count);
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

	uint32_t max_sets = info.max_sets;
	if (max_sets == 0) {
		auto cap = uint64_t(std::numeric_limits <uint32_t> ::max());
		max_sets = uint32_t(std::min(total_sets, cap));
	}

	auto pool_info = vk::DescriptorPoolCreateInfo()
		.setMaxSets(max_sets)
		.setPoolSizes(sizes)
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	return DescriptorPool(device.logical.createDescriptorPool(pool_info));
}

} // namespace rcgp
