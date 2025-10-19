#pragma once

#include "session.hpp"
#include "../msv/stage.hpp"

struct Device {
	vk::Device logical;
	vk::PhysicalDevice physical;
	vk::PhysicalDeviceMemoryProperties properties;
	vk::detail::DispatchLoaderDynamic loader;

	// Device construction
	struct Info {
	};

	static auto from(const Session &session, vk::detail::DispatchLoaderDynamic &dld, const Info &info) {
		Device device;

		// Allocate the physical device
		device.physical = session.handle.enumeratePhysicalDevices().front();
		device.properties = device.physical.getMemoryProperties();

		// Allocate the logical device
		auto priority = 1.0f;

		auto device_queues_info = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(0)
			.setQueuePriorities(priority)
			.setQueueCount(1);

		auto device_info = vk::DeviceCreateInfo()
			.setQueueCreateInfos(device_queues_info);

		device.logical = device.physical.createDevice(device_info);

		// Attach to dynamic loader
		dld.init(device.logical);

		return device;
	}
};

// Shader stage compilation
struct Compiler {
	vk::Device &reference;

	template <Stage S, typename R, typename ... Args>
	auto compile(stage <S, R, Args...> shader) {
		static_assert(is_uncompiled_shader_stage(S), "waah");

		return stage <compiled_shader_stage(S), R, Args...> ();
	}

	struct Info {
		// ...
	};

	static auto from(const Device &device, const Info &info) {
	}
};
