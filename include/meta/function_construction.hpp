#pragma once

#include <functional>

#include "../dsl/jems.hpp"
#include "injection_state.hpp"
#include "shader_stage.hpp"
#include "stage_argument_injector.hpp"

template <ShaderStage S, typename R, typename ... Args>
struct signature {
	using args = std::tuple <std::decay_t <Args> ...>;
	using returns = R;
	using type = shader_stage <S, R, Args...>;
};

template <ShaderStage S, typename Rt, typename R, typename ... Args>
constexpr auto new_signature(std::function <R (Args...)>) -> signature <S, Rt, Args...>;

template <ShaderStage S, typename R, typename F>
auto compile(F ftn)
{
	// TODO: require that all arguments are reflected?

	// TODO: custom vertex assembler stage; experiments section showing an alternative pipeline...
	// TODO: ss of enums via wrapper type wrap <Enum> and then extract substring?
	using function = decltype(std::function(ftn));
	using signature = decltype(new_signature <S, R> (std::declval <function> ()));

	typename signature::type result;

	// TODO: verify signature against provided stage...

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

template <ShaderStage, int>
struct _fn_operator {};

template <ShaderStage>
struct _stage_operator {};

#define $stage(S) _stage_operator <ShaderStage::S> () *

#define $vertex		$stage(eVertex)
#define $fragment	$stage(eFragment)
#define $compute	$stage(eCompute)

template <typename ... Args>
struct compact_returns {
	using type = std::tuple <Args...>;
};

template <typename T>
struct compact_returns <T> {
	using type = T;
};

template <>
struct compact_returns <> {
	using type = void;
};

template <typename ... Ts>
using compact_returns_t = compact_returns <Ts...> ::type;

namespace frenj_ret {

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

template <size_t v>
struct Reader {
	friend auto adl_lever(Reader);
};

template <size_t v, typename T>
struct Writer {
	friend auto adl_lever(Reader <v>) {
		return T();
	}
};

void adl_lever();

template <size_t v>
using Read = std::remove_pointer_t <decltype(adl_lever(Reader <v> {}))>;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

} // namespace frenj_ret

#define $returns(...) decltype(frenj_ret::Writer <__COUNTER__, compact_returns_t <__VA_ARGS__>> {}, void())
#define $return (_return_operator <frenj_ret::Read <__rpidx.value>> ()) << 
#define $fn (_fn_operator <ShaderStage::eUndefined, __COUNTER__ + 2> ()) \
	<< [__rpidx = el <__COUNTER__ + 1> ()] $context_capture
#define $cafn(...) (_fn_operator <ShaderStage::eUndefined, __COUNTER__ + 2> ()) \
	<< [__VA_ARGS__ __VA_OPT__(,) __rpidx = el <__COUNTER__ + 1> ()] $context_capture

template <ShaderStage S, int I>
auto operator<<(_fn_operator <S, I>, auto lambda)
{
	using R = frenj_ret::Read <I>;
	return compile <S, R> (lambda);
}

template <ShaderStage S, int I>
auto operator*(_stage_operator <S>, _fn_operator <ShaderStage::eUndefined, I>)
{
	return _fn_operator <S, I> ();
}
