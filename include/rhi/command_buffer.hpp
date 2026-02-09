#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace rcgp {

struct Buffer;
struct Image;

struct CommandBuffer : vk::CommandBuffer {
	using super = vk::CommandBuffer;

	CommandBuffer() = default;
	CommandBuffer(
		const vk::CommandBuffer &cmd,
		const vk::detail::DispatchLoaderDynamic *loader = nullptr
	);

	const vk::detail::DispatchLoaderDynamic *loader = nullptr;

	auto begin() const -> const CommandBuffer &;
	auto begin(const vk::CommandBufferBeginInfo &info) const -> const CommandBuffer &;
	auto transition_image_layout(Image &image, vk::ImageLayout new_layout) const -> const CommandBuffer &;
	auto copy_buffer_to_image(const Buffer &staging, const Image &image) const -> const CommandBuffer &;
	auto copy_image(const Image &src, const Image &dst) const -> const CommandBuffer &;
	auto end() const -> const CommandBuffer &;
	auto draw_mesh_tasks(uint32_t x, uint32_t y = 1, uint32_t z = 1) const -> const CommandBuffer &;
};

} // namespace rcgp
