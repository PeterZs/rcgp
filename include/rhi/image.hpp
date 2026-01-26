#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace rcgp {

struct Image {
	struct Description {
		vk::Extent3D extent {};
		vk::Format format = vk::Format::eUndefined;
		vk::ImageUsageFlags usage {};
		vk::MemoryPropertyFlags properties {};
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
	};

	vk::Device device;
	vk::Image handle;
	vk::DeviceMemory backing;
	vk::ImageView view;
	vk::ImageLayout layout = vk::ImageLayout::eUndefined;
	Description description;

	auto extent() const -> vk::Extent3D {
		return description.extent;
	}

	auto layers() const -> vk::ImageSubresourceLayers {
		return vk::ImageSubresourceLayers()
			.setAspectMask(description.aspect)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);
	}

	auto range() const -> vk::ImageSubresourceRange {
		return vk::ImageSubresourceRange()
			.setAspectMask(description.aspect)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);
	}
	
	vk::DescriptorImageInfo descriptor_info(const vk::Sampler &sampler) const {
		return vk::DescriptorImageInfo()
			.setSampler(sampler)
			.setImageView(view)
			.setImageLayout(layout);
	}

	void destroy();

	static Image from(const Device &device, const Description &info);
};

} // namespace rcgp
