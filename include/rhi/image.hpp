#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "../util/logging.hpp"

struct Image {
	struct Info {
		vk::Extent2D extent {};
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
	Info info;

	vk::ImageMemoryBarrier memory_barrier(
		vk::ImageLayout new_layout,
		vk::AccessFlags src_access = {},
		vk::AccessFlags dst_access = {},
		vk::ImageAspectFlags aspect = {}
	);

	void destroy();

	static Image from(const Device &device, const Info &info);
};
