#pragma once

#include "buffer.hpp"

namespace rcgp {

struct Device;

struct AccelerationStructure {
	vk::AccelerationStructureKHR handle;
	vk::DeviceSize scratch_size = 0;
	Buffer buffer;

	void destroy(const Device &device);

	static AccelerationStructure from(
		const Device &device,
		const vk::AccelerationStructureBuildGeometryInfoKHR &build_info,
		uint32_t max_primitive_count
	);
};

} // namespace rcgp
