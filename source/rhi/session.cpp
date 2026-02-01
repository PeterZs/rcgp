#include <vector>

#include <cstdlib>
#include <iostream>
#include <print>

#include "rhi/session.hpp"
#include "rhi/glfw.hpp"

namespace rcgp {

VKAPI_ATTR VKAPI_CALL
vk::Bool32 general_validation_callback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT types,
	const vk::DebugUtilsMessengerCallbackDataEXT *data,
	void *user_data
)
{
	auto *context = reinterpret_cast <Session *> (user_data);

	if (context->validation_callback)
		context->validation_callback.value()(severity, data->pMessage);
	else
		std::println(std::cerr, "vulkan: {}", data->pMessage);

	bool trap = context->trap_on_error && (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	if (trap)
		std::abort();

	return false;
}

auto Session::from(const Options &options) -> std::tuple <
	std::unique_ptr <Session>,
	vk::detail::DispatchLoaderDynamic
>
{
	auto product = std::tuple {
		std::make_unique <Session> (),
		vk::detail::DispatchLoaderDynamic(),
	};

	auto &[session, dld] = product;

	glfw::boot();

	vk::detail::DynamicLoader dl;
	auto vkGetInstanceProcAddr = dl.getProcAddress
		<PFN_vkGetInstanceProcAddr> ("vkGetInstanceProcAddr");

	dld.init(vkGetInstanceProcAddr);

	auto layers = std::vector <const char *> {};
	if (options.validation)
		layers.emplace_back("VK_LAYER_KHRONOS_validation");

	auto extensions = std::vector <const char *> {
		VK_KHR_SURFACE_EXTENSION_NAME,
	};

	glfw::load_extensions(extensions);
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

	session->trap_on_error = options.trap_on_error;
	session->validation_callback = options.validation_callback;
	auto debug_info = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageType(debug_type_flags)
		.setMessageSeverity(debug_severity_flags)
		.setPfnUserCallback(general_validation_callback)
		.setPUserData(session.get());

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

	session->handle = vk::createInstance(instance_info, nullptr, dld);

	dld.init(session->handle, vkGetInstanceProcAddr);

	if (options.validation)
		session->debugger = session->handle.createDebugUtilsMessengerEXT(debug_info, nullptr, dld);

	return product;
}

} // namespace rcgp
