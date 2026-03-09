#include <vector>

#include <iostream>
#include <print>

#include <GLFW/glfw3.h>

#include "rhi/session.hpp"

namespace rcgp {

void glfw_boot()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void glfw_load_extensions(std::vector <const char *> &list)
{
	uint32_t count = 0;
	const char **extensions = glfwGetRequiredInstanceExtensions(&count);
	list.insert(list.end(), extensions, extensions + count);
}

VKAPI_ATTR VKAPI_CALL
vk::Bool32 general_validation_callback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT types,
	const vk::DebugUtilsMessengerCallbackDataEXT *data,
	void *user_data
)
{
	auto *debugging = reinterpret_cast <Session::Debugging *> (user_data);

	if (debugging->callback.has_value())
		debugging->callback.value()(severity, data->pMessage);
	else
		std::println(std::cerr, "vulkan: {}", data->pMessage);

	bool trap = debugging->trap_on_error && (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	if (trap)
		__builtin_trap();

	return false;
}

auto Session::from(const Options &options) -> std::tuple <
	Session,
	vk::detail::DispatchLoaderDynamic
>
{
	glfw_boot();

	vk::detail::DynamicLoader dl;
	auto vkGetInstanceProcAddr = dl.getProcAddress
		<PFN_vkGetInstanceProcAddr> ("vkGetInstanceProcAddr");

	vk::detail::DispatchLoaderDynamic dld;
	dld.init(vkGetInstanceProcAddr);

	auto layers = std::vector <const char *> {};
	if (options.validation)
		layers.emplace_back("VK_LAYER_KHRONOS_validation");

	auto extensions = std::vector <const char *> {
		VK_KHR_SURFACE_EXTENSION_NAME,
	};

	glfw_load_extensions(extensions);
	if (options.validation) {
		extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	auto debug_severity_flags =
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
		| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
		| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

	auto debug_type_flags =
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
		| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	auto debugging = std::make_shared <Debugging> (
		options.trap_on_error,
		options.validation_callback
	);

	auto debug_info = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageType(debug_type_flags)
		.setMessageSeverity(debug_severity_flags)
		.setPfnUserCallback(general_validation_callback)
		.setPUserData(debugging.get());

	auto app_info = vk::ApplicationInfo()
		.setApiVersion(VK_API_VERSION_1_4)
		.setApplicationVersion(options.application_version)
		.setPApplicationName(options.application_name)
		.setEngineVersion(options.engine_version)
		.setPEngineName(options.engine_name);

	auto validation_features = vk::ValidationFeaturesEXT()
		.setEnabledValidationFeatures(options.validation_features);

	auto instance_info = vk::InstanceCreateInfo()
		.setPApplicationInfo(&app_info)
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions);

	if (options.validation) {
		instance_info.setPNext(&validation_features);
		if (options.validate_instance)
			validation_features.setPNext(&debug_info);
	}

	auto handle = vk::createInstance(instance_info, nullptr, dld);
	dld.init(handle, vkGetInstanceProcAddr);

	vk::DebugUtilsMessengerEXT debugger;
	if (options.validation)
		debugger = handle.createDebugUtilsMessengerEXT(debug_info, nullptr, dld);

	return { Session(handle, debugger, debugging), dld };
}

} // namespace rcgp
