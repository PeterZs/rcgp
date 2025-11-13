#pragma once

#include <functional>

#include "msv/context.hpp"
#include "msv/function_return_injection.hpp"
#include "msv/reference.hpp"
#include "msv/reflection.hpp"
#include "msv/resources.hpp"
#include "msv/stage.hpp"
#include "msv/stage_intrinsics.hpp"
#include "msv/static_string.hpp"
#include "msv/this_injection.hpp"
#include "msv/type_hash.hpp"
#include "msv/type_string_extensions.hpp"

#include "rhi/session.hpp"
#include "rhi/device.hpp"

#include "dsl/tracer.hpp"
#include "dsl/jems.hpp"
#include "dsl/generators/assembly.hpp"
#include "dsl/generators/glsl.hpp"

template <typename R>
struct _return_operator {};

template <typename R, typename U>
void operator<<(_return_operator <R>, const U &value)
{
	static_assert(std::is_convertible_v <U, R>);
}

template <Stage S, typename R, typename ... Args>
struct signature {
	using args = std::tuple <std::decay_t <Args> ...>;
	using returns = R;
	using type = stage <S, R, Args...>;
};

template <Stage S, typename Rt, typename R, typename ... Args>
constexpr auto new_signature(std::function <R (Args...)>) -> signature <S, Rt, Args...>;

// TODO: custom vertex assembler stage; experiments section showing an alternative pipeline...
// TODO: ss of enums via wrapper type wrap <Enum> and then extract substring?

template <Stage S>
void inject_execution_model()
{
	if constexpr (S == Stage::RepresentationalVertex)
		$tsb.context.model = ExecutionModel::eVulkanVertex;
	else
		static_assert(false, "no execution model for stage");
}

template <typename T>
auto reconstruct_type()
{
	// TODO: specialization structures to avoid explicit lists like this...
	if constexpr (std::is_same_v <T, vec2>)
		return jems::type(VectorType <float, 2> ());
	else if constexpr (std::is_same_v <T, mat4>)
		return jems::type(MatrixType <float, 4, 4> ());
	else
		static_assert(false, ($ss("failed to reconstruct type ") + $ss_type(T)).view());
}

template <typename T>
void inject_item(T &item, Reference ref)
{
	if constexpr (std::is_base_of_v <jems::handle, T>)
		item.ref = ref;
	else
		static_assert(false, ($ss("failed to inject reference into item of type ") + $ss_type(T)).view());
}

struct InjectionState {
	size_t argidx;
	size_t threadidx;

	struct Delta {
		bool argument;
		bool thread_input;
	};

	InjectionState next(Delta delta) const {
		return {
			.argidx = argidx + delta.argument,
			.threadidx = threadidx + delta.thread_input,
		};
	}
};

template <Stage S, typename T>
struct stage_argument_injector {};

// For subroutines, normals arguments are normal arguments
template <typename T>
struct stage_argument_injector <Stage::Undefined, T> {
	static auto apply(T &value, const InjectionState &state) {
		auto type = reconstruct_type <T> ();
		auto arg = Argument(type, state.argidx);
		$tsb.context.add_argument(arg);
		inject_item(value, jems::intrinsic(arg));
		return InjectionState::Delta {
			.argument = true,
			.thread_input = false,
		};
	}
};

// For vertex and fragment shaders, normal arguments are thread inputs
template <Stage S, typename T>
requires (S == Stage::RepresentationalVertex || S == Stage::RepresentationalFragment)
struct stage_argument_injector <S, T> {
	static auto apply(T &value, const InjectionState &state) {
		auto type = reconstruct_type <T> ();
		auto tin = ThreadInput(type, state.threadidx);
		$tsb.context.add_thread_input(tin);
		inject_item(value, jems::intrinsic(tin));
		return InjectionState::Delta {
			.argument = false,
			.thread_input = true,
		};
	}
};

// Always ignore the implicit context
template <Stage S, auto & ... refs>
struct stage_argument_injector <S, implicit_context <refs...>> {
	static auto apply(auto &value, const InjectionState &state) {
		return InjectionState::Delta { false, false };
	}
};

template <Stage S, size_t Index, typename ... Args>
void inject_arguments(std::tuple <Args...> &args, const InjectionState &state)
{
	auto &value = std::get <Index> (args);
	using T = std::decay_t <decltype(value)>;

	auto delta = stage_argument_injector <S, T> ::apply(value, state);
	if constexpr (Index + 1 < sizeof...(Args))
		inject_arguments <S, Index + 1> (args, state.next(delta));
}

template <Stage S, typename R, typename F>
auto compile(F ftn)
{
	using function = decltype(std::function(ftn));
	using signature = decltype(new_signature <S, R> (std::declval <function> ()));

	typename signature::type result;

	if (auto s = jems::scope(result)) {
		typename signature::args args;
		inject_execution_model <S> ();
		inject_arguments <S, 0> (args, InjectionState(0, 0));
		// TODO: need to concretize returns at the return operator...
		// (not here or at return value construction)
		std::apply(ftn, args);
	}

	return result;
}

template <Stage, int>
struct _fn_operator {};

template <Stage>
struct _stage_operator {};

#define $stage(S) _stage_operator <Stage::S> () *

#define $vertex		$stage(RepresentationalVertex)
#define $fragment	$stage(RepresentationalFragment)
#define $compute	$stage(RepresentationalCompute)

#define $returns(T) decltype(fn_return_injection::Writer <decltype(_return_proxy), T> {}, void())
#define $return (_return_operator <fn_return_injection::Read <decltype(_return_proxy)> ::unfoil> ()) << 
#define $fn (_fn_operator <Stage::Undefined, __COUNTER__ + 1> ()) << [_return_proxy = fn_return_injection::proxy_tag <__COUNTER__> ()] $context_capture
#define $cafn(...) (_fn_operator <Stage::Undefined, __COUNTER__ + 1> ()) << [__VA_ARGS__ __VA_OPT__(,) _return_proxy = fn_return_injection::proxy_tag <__COUNTER__> ()] $context_capture

template <Stage S, int I>
auto operator<<(_fn_operator <S, I>, auto lambda)
{
	using R = typename fn_return_injection::Read <fn_return_injection::proxy_tag <I>> ::unfoil;
	return compile <S, R> (lambda);
}

template <Stage S, int I>
auto operator*(_stage_operator <S>, _fn_operator <Stage::Undefined, I>)
{
	return _fn_operator <S, I> ();
}
