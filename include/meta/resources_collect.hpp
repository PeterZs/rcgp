#pragma once

#include "contract.hpp"
#include "resources.hpp"
#include "shader_stage_conversion.hpp"

namespace rcgp {

template <auto &ref, ShaderStage ... Ss>
struct stage_wrapper : contract <ref> {
	using stages = std::integer_sequence <ShaderStage, Ss...>;
	using type = reference_base_of <ref>;
	using contract = rcgp::contract <ref>;

	template <ShaderStage S>
	using append_stage = std::conditional_t <
		((Ss == S) || ...),
		stage_wrapper <ref, Ss...>,
		stage_wrapper <ref, Ss..., S>
	>;

	static constexpr auto flags = stage_flags_of <Ss...> ();
};

TYPE_TRAIT(is_push_constant_wrapper);
	template <auto &ref, ShaderStage ... Ss>
	requires (is_push_constant_v <reference_base_of <ref>>)
	TYPE_TRAIT_INCLUDES(is_push_constant_wrapper, stage_wrapper <ref, Ss...>);

TYPE_TRAIT(is_descriptable_wrapper);
	template <auto &ref, ShaderStage ... Ss>
	requires (not is_push_constant_v <reference_base_of <ref>>)
	TYPE_TRAIT_INCLUDES(is_descriptable_wrapper, stage_wrapper <ref, Ss...>);

template <typename List>
using push_constant_resources_t = tlist_filter_t <is_push_constant_wrapper, List>;

template <typename List>
using descriptable_resources_t = tlist_filter_t <is_descriptable_wrapper, List>;

template <typename T>
struct is_attribute_stream_ref : std::false_type {};

template <auto &ref>
struct is_attribute_stream_ref <contract <ref>>
	: std::bool_constant <is_attribute_stream_v <reference_base_of <ref>>> {};

template <typename T>
struct is_global_resource_ref : std::false_type {};

template <auto &ref>
struct is_global_resource_ref <contract <ref>>
	: std::bool_constant <is_global_resource_v <reference_base_of <ref>>> {};

template <ShaderStage S>
struct stage_wrapper_from_ref {
	template <typename T>
	struct apply;

	template <auto &ref>
	struct apply <contract <ref>> {
		using type = stage_wrapper <ref, S>;
	};
};

template <typename IContext>
consteval auto collect_streams(const IContext &)
{
	using refs = typename IContext::tlist;
	using streams = tlist_filter_t <is_attribute_stream_ref, refs>;
	using unique_streams = tlist_unique_t <streams>;
	return unique_streams {};
}

template <ShaderStage S, typename IContext>
consteval auto collect_gvrs(const IContext &)
{
	using refs = typename IContext::tlist;
	using resources = tlist_filter_t <is_global_resource_ref, refs>;
	using wrapped = tlist_transform_t <resources, stage_wrapper_from_ref <S> ::template apply>;
	using unique_wrapped = tlist_unique_t <wrapped>;
	return unique_wrapped {};
}

template <auto &ref, ShaderStage S, typename ... Ts>
consteval auto merge_stage_wrapper(Tlist <Ts...>)
{
	constexpr auto exists = (std::same_as <contract <ref>, contract <Ts::handle>> || ...);
	if constexpr (exists) {
		return Tlist <
			std::conditional_t <
				std::same_as <contract <Ts::handle>, contract <ref>>,
				typename Ts::template append_stage <S>,
				Ts
			> ...
		> {};
	} else {
		return Tlist <Ts..., stage_wrapper <ref, S>> {};
	}
}

template <typename List>
consteval auto merge_stage_wrappers(const List &list)
{
	return list;
}

template <auto &ref, ShaderStage S, typename ... Rest>
consteval auto merge_stage_wrappers(Tlist <stage_wrapper <ref, S>, Rest...>)
{
	auto merged = merge_stage_wrappers(Tlist <Rest...> {});
	return merge_stage_wrapper <ref, S> (merged);
}

} // namespace rcgp
