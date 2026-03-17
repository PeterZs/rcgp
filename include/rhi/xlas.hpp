#pragma once

#include "buffer.hpp"

namespace rcgp {

struct Device;

struct AccelerationStructure {
	vk::AccelerationStructureKHR handle;
	Buffer buffer;

	void destroy(const Device &device);

	static auto from(
		const Device &device,
		const vk::AccelerationStructureBuildGeometryInfoKHR &build_info,
		uint32_t max_primitive_count
	) -> std::tuple <AccelerationStructure, uint32_t>;
};

} // namespace rcgp
