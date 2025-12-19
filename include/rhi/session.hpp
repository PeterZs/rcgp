#pragma once

#include <tuple>
#include <vulkan/vulkan.hpp>

struct Session {
	vk::Instance handle;
	vk::DebugUtilsMessengerEXT debugger;
	bool trap_on_error = true;

	struct Options {
		const std::string &application_name;
		uint32_t application_version = VK_MAKE_VERSION(0, 1, 0);
		const std::string &engine_name;
		uint32_t engine_version = VK_MAKE_VERSION(0, 1, 0);
		bool validation = true;
		bool validate_instance = true;
		bool trap_on_error = true;
		std::vector <vk::ValidationFeatureEnableEXT> validation_features;
	};

	static std::tuple <Session, vk::detail::DispatchLoaderDynamic> from(const Options &info);
};
