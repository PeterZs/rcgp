#pragma once

#include <cstdint>
#include <string_view>

#include <vulkan/vulkan.hpp>

namespace rcgp {

struct Buffer;
struct Image;

struct BarrierDesc {
	vk::PipelineStageFlags2 src_stage  = {};
	vk::AccessFlags2        src_access = {};
	vk::PipelineStageFlags2 dst_stage  = {};
	vk::AccessFlags2        dst_access = {};
};

struct CommandBuffer : vk::CommandBuffer {
	using super = vk::CommandBuffer;

	struct [[maybe_unused]] ScopedLabel {
		const CommandBuffer *command = nullptr;

		ScopedLabel() = default;
		ScopedLabel(
			const CommandBuffer &command_buffer,
			std::string_view name
		);
		ScopedLabel(const ScopedLabel &) = delete;
		ScopedLabel &operator=(const ScopedLabel &) = delete;
		ScopedLabel(ScopedLabel &&other) noexcept;
		ScopedLabel &operator=(ScopedLabel &&other) noexcept;
		~ScopedLabel();
	};

	const vk::detail::DispatchLoaderDynamic *loader = nullptr;

	CommandBuffer() = default;
	CommandBuffer(
		const vk::CommandBuffer &cmd,
		const vk::detail::DispatchLoaderDynamic *loader = nullptr
	);

	void begin() const;
	void begin(const vk::CommandBufferBeginInfo &info) const;
	void transition(Image &image, vk::ImageLayout new_layout, const BarrierDesc &desc = {}) const;
	void copy_buffer_to_image(const Buffer &staging, const Image &image) const;
	void copy_image(const Image &src, const Image &dst) const;
	void end() const;
	void draw_mesh_tasks(uint32_t x, uint32_t y = 1, uint32_t z = 1) const;
	void begin_label(std::string_view name) const;
	void end_label() const;
	[[nodiscard]] ScopedLabel scoped_label(std::string_view name) const;
};

} // namespace rcgp
