#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

struct Image {
	struct Description {
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
	Description description;

	void destroy();

	static Image from(const Device &device, const Description &info);
};
