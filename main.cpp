#include <fmt/printf.h>

#include "ugp/ugp.hpp"

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

int main()
{
	auto session_info = Session::Info {
		.validation_bootstrap = false,
	};

	auto [session, dld] = Session::from(session_info);

	auto device_info = Device::Info();
	auto device = Device::from(session, dld, device_info);

	Tracer::Record record;
	if (auto r = $dsl.begin(record)) {
		auto f = $dsl.constant(1.0f);
		auto g = $dsl.constant(12);
		auto h = $dsl.binary_operation(BinaryOperation::eAdd, f, g);
	}

	fmt::println("# of instructions in record: {}", record.size());
	for (auto &instr : record)
		fmt::println("\t{}", instr);

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
