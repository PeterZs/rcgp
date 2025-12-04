#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "../util/logging.hpp"

struct Image {
	Device device;
	vk::Image handle;
	vk::DeviceMemory backing;
	vk::ImageView view;
	vk::ImageLayout layout = vk::ImageLayout::eUndefined;
	vk::Extent2D extent;
	vk::Format format = vk::Format::eUndefined;
	vk::ImageUsageFlags usage;
	vk::MemoryPropertyFlags properties;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;

	vk::ImageMemoryBarrier memory_barrier(vk::ImageLayout new_layout, vk::AccessFlags src_access = {}, vk::AccessFlags dst_access = {});

	void destroy();

	static Image from(
		const Device &device,
		vk::Extent2D extent,
		vk::Format format,
		vk::ImageUsageFlags usage,
		vk::ImageAspectFlags aspect,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal
	);
};
