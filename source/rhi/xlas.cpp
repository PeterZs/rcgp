#include "rhi/xlas.hpp"
#include "rhi/device.hpp"

namespace rcgp {

auto AccelerationStructure::from(
	const Device &device,
	const vk::AccelerationStructureBuildGeometryInfoKHR &build_info,
	uint32_t max_primitive_count
) -> std::tuple <AccelerationStructure, uint32_t>
{
	auto sizes = device.logical.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		build_info, max_primitive_count, device.loader
	);

	AccelerationStructure result;
	result.buffer = Buffer::from(device,
		sizes.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
		       | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	result.handle = device.logical.createAccelerationStructureKHR(
		vk::AccelerationStructureCreateInfoKHR()
			.setBuffer(result.buffer.handle)
			.setSize(sizes.accelerationStructureSize)
			.setType(build_info.type),
		nullptr, device.loader
	);

	return std::tuple { result, sizes.buildScratchSize };
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
