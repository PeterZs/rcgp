#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace rcgp {

using ValidationCallback = std::function <void (vk::DebugUtilsMessageSeverityFlagBitsEXT, std::string_view)>;

struct Session {
	vk::Instance handle;
	vk::DebugUtilsMessengerEXT debugger;

	struct Debugging {
		bool trap_on_error = true;
		std::optional <ValidationCallback> callback;
	};

	std::shared_ptr <Debugging> debugging;

	void destroy() {
		handle.destroy();
	}

	struct Options {
		const char *const application_name;
		uint32_t application_version = VK_MAKE_VERSION(0, 1, 0);
		const char *const engine_name;
		uint32_t engine_version = VK_MAKE_VERSION(0, 1, 0);
		bool validation = true;
		bool validate_instance = true;
		bool trap_on_error = true;
		std::optional <ValidationCallback> validation_callback;
		std::vector <vk::ValidationFeatureEnableEXT> validation_features;
	};

	static auto from(const Options &info) -> std::tuple <
		Session,
		vk::detail::DispatchLoaderDynamic
	>;
};

} // namespace rcgp
