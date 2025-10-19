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

#define $returns(T) decltype(fn_return_injection::Writer <decltype(_return_proxy), T> {}, void())
#define $return (_return_operator <fn_return_injection::Read <decltype(_return_proxy)> ::unfoil> ()) << 
#define $fn (_fn_operator <Stage::Undefined, __COUNTER__ + 1> ()) << [_return_proxy = fn_return_injection::proxy_tag <__COUNTER__> ()] $context_capture
#define $cafn(...) (_fn_operator <Stage::Undefined, __COUNTER__ + 1> ()) << [__VA_ARGS__ __VA_OPT__(,) _return_proxy = fn_return_injection::proxy_tag <__COUNTER__> ()] $context_capture

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

// TODO: context creation and invocation: $context(method)(...)
// the context becomes an operator which removes implicit things (i.e. resource references)
// from the type signature of the given function

// struct Backend {
// };

#include <map>

namespace generators {

struct Assembly {
	const Block &block;

	std::map <std::intptr_t, uint32_t> ids;

	std::string stringify(Reference ref) {
		auto addr = intptr_t(ref.get());

		auto it = ids.find(addr);
		if (it != ids.end()) {
			return fmt::format("${}", it->second);
		} else {
			auto id = ids.size();
			ids[addr] = id;
			return fmt::format("${}", id);
		}
	}

	#define $assign stringify(ref) + " = " +

	std::string stringify(Constant x, Reference ref) {
		return $assign std::visit([](auto x) {
			return fmt::format("{}", x);
		}, x);
	}
	
	std::string stringify_impl(PrimitiveType x, Reference ref) {
		vswitch(x) {
		vcase(bool): return "bool";
		vcase(int32_t): return "i32";
		vcase(uint32_t): return "u32";
		vcase(float): return "f32";
		vcase(VectorType <float, 2>): return "vec2";
		vcase(VectorType <float, 3>): return "vec3";
		vcase(VectorType <float, 4>): return "vec4";
		default:
			break;
		}

		return "primitive(?)";
	}
	
	std::string stringify(Type x, Reference ref) {
		return $assign std::visit([&](auto x) {
			return stringify_impl(x, ref);
		}, x);
	}

	std::string stringify(Operation x, Reference ref) {
		std::string op = "?";
		switch (x.code) {
		case Operation::eAdd: op = "add"; break;
		default:
			break;
		}

		return $assign fmt::format("{}({}, {})",
			op, stringify(x.a), stringify(x.b));
	}
	
	std::string stringify(Store x, Reference ref) {
		return fmt::format("store {} {}",
			stringify(x.destination), stringify(x.source));
	}

	std::string stringify(Intrinsic x, Reference ref) {
		switch (x.code) {
		case Intrinsic::eSVPosition: return $assign "SVPosition";
		default:
			break;
		}
		
		return $assign "?";
	}

	std::string stringify(Construct x, Reference ref) {
		std::string result = fmt::format("new {}(", stringify(x.type));
		for (size_t i = 0; i < x.args.size(); i++) {
			result += stringify(x.args[i]);
			if (i + 1 < x.args.size())
				result += ", ";
		}
		result += ")";

		return $assign result;
	}

	#undef $assign

	std::string stringify(auto x, Reference ref) {
		return "?";
	}

	void display(FILE *stream = stdout, size_t tabs = 0) {
		fmt::println("block() {{");
		for (auto &instr : block) {
			auto str = std::visit([&](auto x) {
				return stringify(x, instr);
			}, *instr);
			auto loc = instr->debug_info.origin;
			auto rel = std::filesystem::relative(loc.file_name());
			fmt::println("\t{:<30} ; from {}:{}", str, rel.string(), loc.line());
		}
		fmt::println("}}");
	}
};

} // namespace generators

template <int>
struct A {};

struct X {
	int32_t value;
	[[no_unique_address]] A <0> a1;
	[[no_unique_address]] A <1> a2;
	[[no_unique_address]] A <2> a3;
};

static_assert(sizeof(X) == 4);

int main()
{
	auto session_info = Session::Info {
		.validation_bootstrap = false,
	};

	auto [session, dld] = Session::from(session_info);

	auto device_info = Device::Info();
	auto device = Device::from(session, dld, device_info);

	// NOTE: we actually cant compile the shaders until the we understand the whole pipeline...
	// defer device supplication to the pipeline point?
	auto vs = $vertex $fn(Topology::Triangle <vec2> pos, $use(mvp)) -> $returns(Position) {
		$return Position(vec4(pos, 0, 1));
	};

	generators::Assembly(vs).display();

	// vs.display_assembly();

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
