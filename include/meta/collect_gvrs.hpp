#pragma once

#include "implicit_context.hpp"
#include "reference.hpp"
#include "resources.hpp"
#include "shader_stage.hpp"

// TODO: refactor...
template <auto &ref, ShaderStage ... Ss>
struct stage_wrapper {
	using stages = std::integer_sequence <ShaderStage, Ss...>;
	using type = reference_base_t <ref>;
	using reference = reference <ref>;

	// TODO: get rid of this...?
	template <ShaderStage S>
	using append_stage = std::conditional_t <
		((Ss == S) || ...),
		stage_wrapper <ref, Ss...>,
		stage_wrapper <ref, Ss..., S>
	>;

	static constexpr auto flags = stage_flags_of <Ss...> ();
};

template <ShaderStage S, auto &ref, typename ... Ts>
auto add_gvr(const Tlist <Ts...> &in)
{
	using base = reference <ref> ::base;
	if constexpr (not is_global_resource_v <base>) {
		return in;
	} else {
		constexpr auto exists = (std::same_as <typename Ts::reference, reference <ref>> || ...);

		if constexpr (exists) {
			return Tlist <
				std::conditional_t <
					std::same_as <typename Ts::reference, reference <ref>>,
					typename Ts::template append_stage <S>,
					Ts
				> ...
			> {};
		} else {
			return Tlist <Ts..., stage_wrapper <ref, S>> {};
		}
	}
}

template <ShaderStage S, typename ... Ts>
auto add_gvr_from_implicit_context(const Tlist <Ts...> &in, const implicit_context <> &)
{
	// Empty implicit context means no additional global resources.
	return in;
}

template <ShaderStage S, typename ... Ts, auto &b, auto &...bs>
auto add_gvr_from_implicit_context(const Tlist <Ts...> &in, const implicit_context <b, bs...> &)
{
	auto out = add_gvr <S, b> (in);
	if constexpr (sizeof...(bs))
		return add_gvr_from_implicit_context <S> (out, implicit_context <bs...> ());
	else
		return out;
}
