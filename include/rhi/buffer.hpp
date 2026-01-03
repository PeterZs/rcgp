#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

struct Buffer {
	vk::Device device;
	vk::Buffer handle;
	vk::DeviceMemory backing;
	vk::DeviceSize offset = 0;
	vk::DeviceSize size = 0;
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags properties;

	auto write(
		const void *data,
		size_t bytes,
		vk::DeviceSize relative_offset = 0
	) const -> const Buffer &;
	
	template <typename U>
	auto &write(std::span <U> memory, vk::DeviceSize offset = 0) const {
		return write(memory.data(), memory.size_bytes(), offset);
	}
	
	void destroy();

	static auto from(
		const Device &device,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties
	) -> Buffer;
};

// TODO: arena allocator for buffers
