#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

struct Buffer {
	Device device;
	vk::Buffer handle;
	vk::DeviceMemory backing;
	vk::DeviceSize offset = 0;
	vk::DeviceSize size = 0;
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags properties;

	void write(const void *data, size_t bytes, vk::DeviceSize relative_offset = 0) const;
	void destroy();

	static Buffer from(
		const Device &device,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties
	);
};

// struct BufferArena {
// 	Buffer buffer;
// 	vk::DeviceSize head = 0;
// 	vk::DeviceSize alignment = 256;
// 	std::vector <Buffer> slices;
//
// 	BufferArena() = default;
// 	explicit BufferArena(Buffer backing, vk::DeviceSize alignment = 256);
//
// 	Buffer allocate(vk::DeviceSize bytes);
// };
