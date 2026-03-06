#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

#include "rhi/buffer.hpp"
#include "rhi/command_buffer.hpp"
#include "rhi/image.hpp"

namespace rcgp {

static uint64_t hash_label(std::string_view label)
{
	constexpr uint64_t fnv_offset_basis = 14695981039346656037ull;
	constexpr uint64_t fnv_prime = 1099511628211ull;

	auto hash = fnv_offset_basis;
	for (unsigned char c : label) {
		hash ^= uint64_t(c);
		hash *= fnv_prime;
	}
	return hash;
}

// TODO: separate file
static std::array <float, 4> hsl_to_rgba(float hue_degrees, float saturation, float lightness)
{
	float h = std::fmod(hue_degrees, 360.0f);
	if (h < 0.0f)
		h += 360.0f;

	float chroma = (1.0f - std::abs(2.0f * lightness - 1.0f)) * saturation;
	float segment = h / 60.0f;
	float x = chroma * (1.0f - std::abs(std::fmod(segment, 2.0f) - 1.0f));

	float r1 = 0.0f;
	float g1 = 0.0f;
	float b1 = 0.0f;

	if (segment < 1.0f) {
		r1 = chroma;
		g1 = x;
	} else if (segment < 2.0f) {
		r1 = x;
		g1 = chroma;
	} else if (segment < 3.0f) {
		g1 = chroma;
		b1 = x;
	} else if (segment < 4.0f) {
		g1 = x;
		b1 = chroma;
	} else if (segment < 5.0f) {
		r1 = x;
		b1 = chroma;
	} else {
		r1 = chroma;
		b1 = x;
	}

	float m = lightness - 0.5f * chroma;
	return {
		std::clamp(r1 + m, 0.0f, 1.0f),
		std::clamp(g1 + m, 0.0f, 1.0f),
		std::clamp(b1 + m, 0.0f, 1.0f),
		1.0f
	};
}

static std::array <float, 4> label_color(std::string_view label)
{
	// Stable, vivid palette via fixed S/L and hash-derived hue.
	constexpr float saturation = 0.68f;
	constexpr float lightness = 0.57f;

	auto hash = hash_label(label);
	float hue = float(hash % 36000ull) * 0.01f;
	return hsl_to_rgba(hue, saturation, lightness);
}

static auto layout_to_stage_and_access(vk::ImageLayout layout)
	-> std::pair <vk::PipelineStageFlags2, vk::AccessFlags2>
{
	using enum vk::ImageLayout;

	switch (layout) {
	case eTransferSrcOptimal:
		return {
			vk::PipelineStageFlagBits2::eTransfer,
			vk::AccessFlagBits2::eTransferRead
		};
	case eTransferDstOptimal:
		return {
			vk::PipelineStageFlagBits2::eTransfer,
			vk::AccessFlagBits2::eTransferWrite
		};
	case eColorAttachmentOptimal:
		return {
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite
		};
	case eDepthStencilAttachmentOptimal:
		return {
			vk::PipelineStageFlagBits2::eEarlyFragmentTests
			| vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite
		};
	case ePresentSrcKHR:
		return {
			vk::PipelineStageFlagBits2::eBottomOfPipe,
			vk::AccessFlagBits2::eNone
		};
	case eShaderReadOnlyOptimal:
		return {
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderRead
		};
	default:
		return {
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone
		};
	}
}

static auto decompose_layout_transition(
	vk::ImageLayout old_layout,
	vk::ImageLayout new_layout
)
{
	auto [src_stage, src_access] = layout_to_stage_and_access(old_layout);
	auto [dst_stage, dst_access] = layout_to_stage_and_access(new_layout);
	return std::tuple(src_stage, src_access, dst_stage, dst_access);
}

CommandBuffer::CommandBuffer(
	const vk::CommandBuffer &cmd,
	const vk::detail::DispatchLoaderDynamic *loader_
) : vk::CommandBuffer(cmd), loader(loader_)
{}

CommandBuffer::ScopedLabel::ScopedLabel(
	const CommandBuffer &command_buffer,
	std::string_view name
) : command(&command_buffer)
{
	command->begin_label(name);
}

CommandBuffer::ScopedLabel::ScopedLabel(ScopedLabel &&other) noexcept
	: command(std::exchange(other.command, nullptr))
{}

CommandBuffer::ScopedLabel &CommandBuffer::ScopedLabel::operator=(ScopedLabel &&other) noexcept
{
	if (this == &other)
		return *this;
	if (command != nullptr)
		command->end_label();
	command = std::exchange(other.command, nullptr);
	return *this;
}

CommandBuffer::ScopedLabel::~ScopedLabel()
{
	if (command != nullptr)
		command->end_label();
}

void CommandBuffer::begin() const
{
	super::begin(vk::CommandBufferBeginInfo());
}

void CommandBuffer::begin(const vk::CommandBufferBeginInfo &info) const
{
	super::begin(info);
}

void CommandBuffer::transition(Image &image, vk::ImageLayout new_layout, const BarrierDesc &desc) const
{
	vk::PipelineStageFlags2 src_stage  = desc.src_stage;
	vk::AccessFlags2        src_access = desc.src_access;
	vk::PipelineStageFlags2 dst_stage  = desc.dst_stage;
	vk::AccessFlags2        dst_access = desc.dst_access;

	if (!src_stage && !dst_stage) {
		auto [ss, sa, ds, da] = decompose_layout_transition(image.layout, new_layout);
		src_stage = ss; src_access = sa;
		dst_stage = ds; dst_access = da;
	}

	auto barrier = vk::ImageMemoryBarrier2()
		.setImage(image.handle)
		.setOldLayout(image.layout)
		.setNewLayout(new_layout)
		.setSrcStageMask(src_stage)
		.setSrcAccessMask(src_access)
		.setDstStageMask(dst_stage)
		.setDstAccessMask(dst_access)
		.setSubresourceRange(image.range());

	pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));

	image.layout = new_layout;
}

void CommandBuffer::copy_buffer_to_image(const Buffer &staging, const Image &image) const
{
	auto copy = vk::BufferImageCopy()
		.setImageSubresource(image.layers())
		.setImageExtent(image.extent());

	super::copyBufferToImage(staging.handle, image.handle, image.layout, copy);
}

void CommandBuffer::copy_image(const Image &src, const Image &dst) const
{
	auto copy = vk::ImageCopy()
		.setSrcSubresource(src.layers())
		.setDstSubresource(dst.layers())
		.setExtent(src.extent());

	super::copyImage(
		src.handle,
		src.layout,
		dst.handle,
		dst.layout,
		copy
	);
}

void CommandBuffer::end() const
{
	super::end();
}

void CommandBuffer::draw_mesh_tasks(uint32_t x, uint32_t y, uint32_t z) const
{
	if (loader == nullptr) {
		std::fputs("draw_mesh_tasks requires a dynamic loader\n", stderr);
		std::abort();
	}
	loader->vkCmdDrawMeshTasksEXT(*this, x, y, z);
}

void CommandBuffer::begin_label(std::string_view name) const
{
	if (loader == nullptr || loader->vkCmdBeginDebugUtilsLabelEXT == nullptr)
		return;

	auto color = label_color(name);
	auto name_storage = std::string(name);
	auto info = vk::DebugUtilsLabelEXT()
		.setPLabelName(name_storage.c_str())
		.setColor(color);
	super::beginDebugUtilsLabelEXT(info, *loader);
}

void CommandBuffer::end_label() const
{
	if (loader == nullptr || loader->vkCmdEndDebugUtilsLabelEXT == nullptr)
		return;
	super::endDebugUtilsLabelEXT(*loader);
}

CommandBuffer::ScopedLabel CommandBuffer::scoped_label(std::string_view name) const
{
	return ScopedLabel(*this, name);
}

} // namespace rcgp
