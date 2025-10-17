#include <fmt/printf.h>

#include "ugp.hpp"

struct VertexAttributes {
	Topology::Triangle <vec3> position;
	Topology::Triangle <vec3> normal;
	Topology::Triangle <vec3> uv;

	// TODO: intrinscs must be listed as members... (or parameters...)

	$reflection(position, normal, uv);
};

struct RasterForward {
	Position svpos;
	Interpolant::Smooth <vec3> position;
	Interpolant::Smooth <vec3> normal;
	Interpolant::Smooth <vec2> uv;

	$reflection(svpos, position, normal, uv);
};

// RasterForward == FragmentAttributes after discarding Position
// and intrinscs on both sides and including order
struct FragmentAttributes {
	vec3 position;
	vec3 normal;
	vec2 uv;

	// NOTE: Interpolation qualifiers are inferred (should not necessary be re-declared here...)

	$reflection(position, normal, uv);
};

struct MVP {
	mat4 model;
	mat4 view;
	mat4 proj;

	$reflection(model, view, proj);
};

ParameterBlock <MVP> mvp;

struct Sampler2D {
	vec4 sample(vec2 uv) {
		return vec4();
	}

	// TODO: disable copies, etc. unique placeholder ID per instance...
};

struct PBRTextures {
	Sampler2D diffuse;
	Sampler2D specular;
	
	$reflection(diffuse, specular);
};

// TODO: index overwrite using layout... same for vertex input attributes...
ParameterBlock <PBRTextures> textures;

template <Stage, int>
struct _fn_operator {};

template <Stage>
struct _stage_operator {};

#define $stage(S) _stage_operator <Stage::S> () *

#define $vertex		$stage(UncompiledVertex)
#define $fragment	$stage(UncompiledFragment)
#define $compute	$stage(UncompiledCompute)

#define $returns(T) decltype(fn_return_injection::Writer <decltype(_return_proxy), RasterForward> {}, void())
#define $return (_return_operator <fn_return_injection::Read <decltype(_return_proxy)> ::unfoil> ()) << 
#define $fn (_fn_operator <Stage::Undefined, __COUNTER__ + 1> ()) << [_return_proxy = fn_return_injection::proxy_tag <__COUNTER__> ()]
#define $cafn(...) (_fn_operator <Stage::Undefined, __COUNTER__ + 1> ()) << [__VA_ARGS__ __VA_OPT__(,) _return_proxy = fn_return_injection::proxy_tag <__COUNTER__> ()]

template <Stage S, int I>
auto operator<<(_fn_operator <S, I>, auto lambda)
{
	using R = typename fn_return_injection::Read <fn_return_injection::proxy_tag <I>> ::unfoil;
	return (_def_operator <S, R> ()) << lambda;
}

template <Stage S, int I>
auto operator*(_stage_operator <S>, _fn_operator <Stage::Undefined, I>)
{
	return _fn_operator <S, I> ();
}

// TODO: compile to SPIRV or whatever when supplied the right device?
// pre-stage (vk::Device >>= UncompiledX) -> X?
// device: ugp::Device
// device.compile(z) -> compiled z
auto z = $vertex $fn(i32 x, $use(mvp)) -> $returns(RasterForward)
{
	$return RasterForward {
		.svpos = vec4(),
		.position = vec3(),
		.normal = vec3(),
		.uv = vec2(),
	};
};

auto f(int y)
{
	return $compute $cafn(&)(i32 x) -> $returns(RasterForward)
	{
		int ww = y;

		$return RasterForward {
			.svpos = vec4(),
			.position = vec3(),
			.normal = vec3(),
			.uv = vec2(),
		};
	};
}

// template <typename A>
// struct ensure_stage {
// 	static_assert(is_stage <A> ::value, "baah");
// };

template <Stage S1, Stage S2>
struct combinator  {
	// NOTE: general strategy is to do the verifications are various
	// derived types doing part of the checks...
	template <
		typename R1, typename ... Args1,
		typename R2, typename ... Args2
	>
	static auto yield(
		const stage <S1, R1, Args1...> &a,
		const stage <S2, R2, Args2...> &b
	) {
		static_assert(false, "waah");
		return a;
	}
};

// Fetch and apply stage combinator
template <
	Stage S1, typename R1, typename ... Args1,
	Stage S2, typename R2, typename ... Args2
>
auto operator>>=(stage <S1, R1, Args1...> a, stage <S2, R2, Args2...> b)
{
	return combinator <S1, S2> ::yield(a, b);
}

// combinators consists of boundary rules and a (sometimes logical) fusion process

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

namespace glfw {

void boot()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void load_extensions(std::vector <const char *> &list)
{
	uint32_t count;
	const char **extensions = glfwGetRequiredInstanceExtensions(&count);
	list.insert(list.end(), extensions, extensions + count);
}

}

VKAPI_ATTR VKAPI_CALL vk::Bool32 validation_callback
(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT types,
	const vk::DebugUtilsMessengerCallbackDataEXT *data,
	void *_
)
{
	fmt::println(stderr, "[vulkan] {}", data->pMessage);
	return false;
}

struct Session {
	vk::Instance handle;
	vk::DebugUtilsMessengerEXT debugger;

	struct Config {
		bool validation = true;
		bool validation_bootstrap = true;
	};

	static auto from(const Config &config) {
		Session session;

		glfw::boot();

		// Configure dynamic dispatch loader
		vk::detail::DynamicLoader dl;
		auto vkGetInstanceProcAddr = dl.getProcAddress
			<PFN_vkGetInstanceProcAddr> ("vkGetInstanceProcAddr");

		vk::detail::DispatchLoaderDynamic dld;
		dld.init(vkGetInstanceProcAddr);

		// Configure layers
		auto layers = std::vector <const char *> {};
		if (config.validation)
			layers.emplace_back("VK_LAYER_KHRONOS_validation");

		// Configure extensions
		auto extensions = std::vector <const char *> {
			VK_KHR_SURFACE_EXTENSION_NAME,
		};

		glfw::load_extensions(extensions);
		if (config.validation) {
			extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Configure the validation debugger
		auto debug_severity_flags = // Everything...
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

		auto debug_type_flags = // Everything...
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

		auto debug_info = vk::DebugUtilsMessengerCreateInfoEXT()
			.setMessageType(debug_type_flags)
			.setMessageSeverity(debug_severity_flags)
			.setPfnUserCallback(validation_callback);

		// Finally, instance creation
		auto app_info = vk::ApplicationInfo()
			.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
			.setApiVersion(VK_API_VERSION_1_4)
			.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
			.setPApplicationName("ugp")
			.setPEngineName("ugp");

		auto instance_info = vk::InstanceCreateInfo()
			.setPApplicationInfo(&app_info)
			.setPEnabledLayerNames(layers)
			.setPEnabledExtensionNames(extensions);

		if (config.validation && config.validation_bootstrap)
			instance_info.setPNext(&debug_info);

		session.handle = vk::createInstance(instance_info, nullptr, dld);

		dld.init(session.handle, vkGetInstanceProcAddr);

		// If needed configure the validation layer messenger
		if (config.validation)
			session.debugger = session.handle.createDebugUtilsMessengerEXT(debug_info, nullptr, dld);

		// TODO: also return the dynamic loader prepped with the instance...
		return std::make_tuple(session, dld);
	}
};

struct Device {
	vk::Device logical;
	vk::PhysicalDevice physical;
	vk::PhysicalDeviceMemoryProperties properties;
	// TODO: dynamic loader...

	// // Shader stage compilation
	// template <Stage S, typename R, typename ... Args>
	// auto compile(stage <S, R, Args...> shader) {
	// 	static_assert(is_uncompiled_shader_stage(S), "waah");
	//
	// 	return stage <compiled_shader_stage(S), R, Args...> ();
	// }
};

int main()
{
	auto config = Session::Config();
	auto [session, dld] = Session::from(config);

	// TODO: how to transport device?
	// auto w = f(10);
	// fuse(12, 14);
	// auto x = z >>= z >>= z >>= w;
	
	// TODO: static_reference as a basic unit for testing reference identity
	// static_assert(std::is_same_v <R1, R2>);
	// static_assert(std::is_same_v <R1, R3>,
	//        $ss_format(
	// 		$ss("see the following diagnostic from Javelin:\n\n"
	//        			"\tJavelin: blah blah blah: {}\n"),
	//        		$ss_type_indented(VertexAttributes::reflection, 1)
	//    	).view()
	// );
}
