#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace rcgp {

struct Buffer;
struct Image;

struct CommandBuffer : vk::CommandBuffer {
	using super = vk::CommandBuffer;

	const vk::detail::DispatchLoaderDynamic *loader = nullptr;

	CommandBuffer() = default;
	CommandBuffer(
		const vk::CommandBuffer &cmd,
		const vk::detail::DispatchLoaderDynamic *loader = nullptr
	);

	void begin() const;
	void begin(const vk::CommandBufferBeginInfo &info) const;
	void transition_image_layout(Image &image, vk::ImageLayout new_layout) const;
	void copy_buffer_to_image(const Buffer &staging, const Image &image) const;
	void copy_image(const Image &src, const Image &dst) const;
	void end() const;
	void draw_mesh_tasks(uint32_t x, uint32_t y = 1, uint32_t z = 1) const;
};

} // namespace rcgp
