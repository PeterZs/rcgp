#include <vector>

#include <fmt/printf.h>
#include <fmt/color.h>

#include "rhi/session.hpp"
#include "rhi/glfw.hpp"

namespace rcgp {

VKAPI_ATTR VKAPI_CALL
vk::Bool32 validation_callback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT types,
	const vk::DebugUtilsMessengerCallbackDataEXT *data,
	void *user_data
)
{
	const bool trap_on_error = reinterpret_cast <std::intptr_t> (user_data);

	auto fg = fmt::fg(fmt::color::gray);
	bool trap = false;

	switch (severity) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		fg = fmt::fg(fmt::color::red);
		trap = trap_on_error;
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		fg = fmt::fg(fmt::color::yellow);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		fg = fmt::fg(fmt::color::light_blue);
		break;
	default:
		break;
	}

	auto header = fmt::format(fmt::emphasis::bold | fg, "[vvl]");
	auto message = fmt::format(fmt::emphasis::faint, "{}", data->pMessage);

	fmt::println(stderr, "{} {}", header, message);
	if (trap)
		__builtin_trap();

	return false;
}

std::tuple <Session, vk::detail::DispatchLoaderDynamic> Session::from(const Options &options)
{
	auto product = std::tuple <Session, vk::detail::DispatchLoaderDynamic> {};
	auto &[session, dld] = product;

	session.trap_on_error = options.trap_on_error;

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

	std::intptr_t trap_on_error = options.trap_on_error;
	auto debug_info = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageType(debug_type_flags)
		.setMessageSeverity(debug_severity_flags)
		.setPfnUserCallback(validation_callback)
		.setPUserData(reinterpret_cast <void *> (trap_on_error));

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

	session.handle = vk::createInstance(instance_info, nullptr, dld);

	dld.init(session.handle, vkGetInstanceProcAddr);

	if (options.validation)
		session.debugger = session.handle.createDebugUtilsMessengerEXT(debug_info, nullptr, dld);

	return product;
}

} // namespace rcgp
