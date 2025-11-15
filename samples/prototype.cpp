#include <fmt/printf.h>

#include <ugp.hpp>

struct VertexAttributes {
	vec3 position;
	vec3 normal;
	vec2 uv;

	// TODO: intrinscs must be listed as members... (or parameters...)

	$reflection(position, normal, uv);
};

struct RasterForward {
	Position svpos;
	Smooth <vec3> position;
	Smooth <vec3> normal;
	Smooth <vec2> uv;

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

struct VertexGroup {
	MVP mvp;
	vec3 another;

	$reflection(mvp, another);
};

using VVR = VertexGroup::reflection;
using VVRex = expand_reflection <VertexGroup> ::type;

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

// template <Stage S1, Stage S2>
// struct combinator  {
// 	// NOTE: general strategy is to do the verifications are various
// 	// derived types doing part of the checks...
// 	template <
// 		typename R1, typename ... Args1,
// 		typename R2, typename ... Args2
// 	>
// 	static auto yield(
// 		const stage <S1, R1, Args1...> &a,
// 		const stage <S2, R2, Args2...> &b
// 	) {
// 		static_assert(false, "waah");
// 		return a;
// 	}
// };

// combinators consists of boundary rules and a (sometimes logical) fusion process

// TODO: context creation and invocation: $context(method)(...)
// the context becomes an operator which removes implicit things (i.e. resource references)
// from the type signature of the given function

template <int>
struct A {};

struct X {
	int32_t value;
	[[no_unique_address]] A <0> a1;
	[[no_unique_address]] A <1> a2;
	[[no_unique_address]] A <2> a3;
};

static_assert(sizeof(X) == 4);

namespace combinators {

template <Topology T>
struct traditional_graphics_pipeline {
	vk::CullModeFlags culling;
};

template <Topology T>
using tgp = traditional_graphics_pipeline <T>;

} // namespace combinators

// NOTE: Templates are allowed as follows:
// template <int N>
// auto ftn = $fn(std::array <i32, N> x, i32 y[N]) -> $returns(void) {
// };

int main()
{
	// TODO: start rolling out some basic tests...
	auto session_info = Session::Info {
		.validation_bootstrap = false,
	};

	auto [session, dld] = Session::from(session_info);

	auto device_info = Device::Info();
	auto device = Device::from(session, dld, device_info);

	// NOTE: we actually cant compile the shaders until the we understand the whole pipeline...
	// defer device supplication to the pipeline point?
	// no, it still makes sense to pair shaders with device;
	// then multi-gpu programming is easier
	//
	// TODO: technically thread inputs should be in the context... make context in terms of types...

	auto vs = $vertex $fn(vec2 pos, mat4 mat, $use(mvp)) -> $returns(
		Position,
		Smooth <vec3>,
		Flat <i32>
	) {
		$return std::tuple {
			Position(mat * vec4(pos, 0, 1)),
			vec3(pos, 1.4),
			12,
		};
	};

	fmt::println("assembly:");
	fmt::println("{}", generators::Assembly(vs).generate());

	// fmt::println("glsl:");
	// fmt::println("{}", generators::GLSL(vs).generate());
	
	// static_assert(!has_reflection <int> ());
	//
	// using x = reflection_expander <i32> ::type;
	// using y = reflection_expander <MVP> ::type;
	//
	// Block block;
	// if (auto s = jems::scope(block)) {
	// 	auto type = reconstruct_type <MVP> ();
	// 	auto rsrc = jems::intrinsic(GlobalResource(type, GlobalResource::ePushConstant));
	// 	auto field = jems::field_access(rsrc, 1);
	// }
	//
	// fmt::println("assembly:");
	// fmt::println("{}", generators::Assembly(block).generate());
}
