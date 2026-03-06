#include "rhi/xlas.hpp"
#include "rhi/device.hpp"

namespace rcgp {

AccelerationStructure AccelerationStructure::from(
	const Device &device,
	const vk::AccelerationStructureBuildGeometryInfoKHR &build_info,
	uint32_t max_primitive_count
) {
	auto sizes = device.logical.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		build_info, max_primitive_count, device.loader
	);

	AccelerationStructure result;
	result.scratch_size = sizes.buildScratchSize;
	result.buffer = Buffer::from(device, {
		.size = sizes.accelerationStructureSize,
		.usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
		       | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		.properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
		.device_address = true,
	});
	result.handle = device.logical.createAccelerationStructureKHR(
		vk::AccelerationStructureCreateInfoKHR()
			.setBuffer(result.buffer.handle)
			.setSize(sizes.accelerationStructureSize)
			.setType(build_info.type),
		nullptr, device.loader
	);
	return result;
}

void AccelerationStructure::destroy(const Device &device)
{
	if (handle) {
		device.logical.destroyAccelerationStructureKHR(handle, nullptr, device.loader);
		handle = nullptr;
	}
	buffer.destroy();
}

} // namespace rcgp
