#pragma once

#include <vulkan/vulkan.hpp>
#include "buffer.hpp"

namespace rcgp {

struct Device;

struct AccelerationStructure {
	Buffer buffer;
	vk::AccelerationStructureKHR handle;
	vk::DeviceSize scratch_size = 0;

	void destroy(const Device &device);

	static AccelerationStructure from(
		const Device &device,
		const vk::AccelerationStructureBuildGeometryInfoKHR &build_info,
		uint32_t max_primitive_count
	);
};

} // namespace rcgp
