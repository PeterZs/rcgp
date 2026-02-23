#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <utility>

#include "rhi/buffer.hpp"
#include "rhi/command_buffer.hpp"
#include "rhi/image.hpp"

namespace rcgp {

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

void CommandBuffer::begin() const
{
	super::begin(vk::CommandBufferBeginInfo());
}

void CommandBuffer::begin(const vk::CommandBufferBeginInfo &info) const
{
	super::begin(info);
}

void CommandBuffer::transition_image_layout(Image &image, vk::ImageLayout new_layout) const
{
	auto [
		src_stage, src_access,
		dst_stage, dst_access
	] = decompose_layout_transition(image.layout, new_layout);

	auto barrier = vk::ImageMemoryBarrier2()
		.setImage(image.handle)
		.setOldLayout(image.layout)
		.setNewLayout(new_layout)
		.setSrcStageMask(src_stage)
		.setDstStageMask(dst_stage)
		.setSrcAccessMask(src_access)
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

} // namespace rcgp
