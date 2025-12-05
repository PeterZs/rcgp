#include "rhi/image.hpp"

vk::ImageMemoryBarrier Image::memory_barrier(vk::ImageLayout new_layout, vk::AccessFlags src_access, vk::AccessFlags dst_access, vk::ImageAspectFlags aspect)
{
	vk::ImageMemoryBarrier barrier;
	barrier.setImage(handle)
		.setOldLayout(layout)
		.setNewLayout(new_layout)
		.setSrcAccessMask(src_access)
		.setDstAccessMask(dst_access)
		.setSubresourceRange(
			vk::ImageSubresourceRange()
				.setAspectMask(aspect)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1)
		);
	layout = new_layout;
	return barrier;
}

void Image::destroy()
{
	if (view) {
		device.logical.destroyImageView(view);
		view = nullptr;
	}

	if (backing) {
		device.logical.destroyImage(handle);
		device.logical.freeMemory(backing);
		handle = nullptr;
		backing = nullptr;
		layout = vk::ImageLayout::eUndefined;
	}
}

Image Image::from(
	const Device &device,
	vk::Extent2D extent,
	vk::Format format,
	vk::ImageUsageFlags usage,
	vk::ImageAspectFlags aspect,
	vk::MemoryPropertyFlags properties,
	vk::ImageTiling tiling)
{
	Image result;
	result.device = device;
	result.extent = extent;
	result.format = format;
	result.usage = usage;
	result.properties = properties;
	result.tiling = tiling;

	auto image_info = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(extent.width, extent.height, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(tiling)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setInitialLayout(result.layout);

	result.handle = device.logical.createImage(image_info);

	auto requirements = device.logical.getImageMemoryRequirements(result.handle);
	auto memory_index = device.find_memory_type(requirements.memoryTypeBits, properties);
	auto memory_info = vk::MemoryAllocateInfo()
		.setAllocationSize(requirements.size)
		.setMemoryTypeIndex(memory_index);

	result.backing = device.logical.allocateMemory(memory_info);
	device.logical.bindImageMemory(result.handle, result.backing, 0);

	auto view_info = vk::ImageViewCreateInfo()
		.setImage(result.handle)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(
			vk::ImageSubresourceRange()
				.setAspectMask(aspect)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1)
		)
		.setFormat(format);

	result.view = device.logical.createImageView(view_info);

	return result;
}
