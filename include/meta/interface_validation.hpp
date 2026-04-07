#pragma once

#include "shader_stage.hpp"
#include "intrinsics_vertex.hpp"
#include "intrinsics_mesh.hpp"
#include "intrinsics_raytracing.hpp"

namespace rcgp {

// Unwrap interpolation qualifiers from vertex/mesh return types
template <typename T>
struct unwrap_output { using type = Tlist <T>; };

template <>
struct unwrap_output <void> { using type = Tlist <>; };

template <builtin T, RateProperties P>
struct unwrap_output <Interpolant <T, P>> { using type = Tlist <T>; };

template <builtin T>
struct unwrap_output <Smooth <T>> { using type = Tlist <T>; };

template <builtin T>
struct unwrap_output <Flat <T>> { using type = Tlist <T>; };

template <builtin T>
struct unwrap_output <NoPerspective <T>> { using type = Tlist <T>; };

template <typename ... Ts>
struct unwrap_output <std::tuple <Ts...>> {
	using type = tlist_concat_t <typename unwrap_output <Ts> ::type...>;
};

template <typename T>
using unwrap_output_t = typename unwrap_output <T> ::type;

// Filter stage IO types (traced params) from shader args
template <typename T>
constexpr bool is_stage_io_v =
	!is_implicit_context_v <T>
	and !is_contract_v <T>
	and !std::is_same_v <T, jems::null>
	and !is_read_only_intrinsic_v <T>
	and !is_write_only_intrinsic_v <T>
	and !is_workgroup_v <T>
	and !is_task_payload_v <T>;

template <typename T>
struct is_stage_io : std::bool_constant <is_stage_io_v <T>> {};

template <typename Shader>
using stage_inputs_t = tlist_filter_t <is_stage_io, typename Shader::args>;

// Vertex -> Fragment: outputs must match inputs
template <typename VertexShader, typename FragmentShader>
constexpr bool vertex_fragment_io_compatible =
	std::is_same_v <
		unwrap_output_t <typename VertexShader::return_type>,
		stage_inputs_t <FragmentShader>
	>;

// Task -> Mesh: payload types must match
template <typename T>
struct extract_task_payload { using type = void; };

template <typename T>
struct extract_task_payload <TaskPayload <T>> { using type = T; };

template <typename Shader>
using task_payload_from_return_t =
	typename extract_task_payload <typename Shader::return_type> ::type;

template <typename Shader>
using task_payload_from_args_t = typename extract_task_payload <
	typename tlist_filter_t <is_task_payload, typename Shader::args> ::template get <0>
> ::type;

template <typename TaskShader, typename MeshShader>
constexpr bool task_mesh_payload_compatible =
	std::is_same_v <
		task_payload_from_return_t <TaskShader>,
		task_payload_from_args_t <MeshShader>
	>;

// Mesh -> Fragment: outputs must match inputs
template <typename T>
struct meshlet_output_types { using type = Tlist <>; };

template <MeshPrimitive P, uint32_t MaxV, uint32_t MaxP, user_defined T>
struct meshlet_output_types <MeshletPayload <P, MaxV, MaxP, T>> {
	template <typename F>
	struct element_of { using type = typename F::element_type; };

	using type = tlist_transform_t <typename T::fields, element_of>;
};

template <typename Shader>
using mesh_outputs_t =
	typename meshlet_output_types <typename Shader::return_type> ::type;

template <typename MeshShader, typename FragmentShader>
constexpr bool mesh_fragment_io_compatible =
	std::is_same_v <mesh_outputs_t <MeshShader>, stage_inputs_t <FragmentShader>>;

// Raytracing: extract dispatcher/receiver trace group addresses from contracts
template <typename T>
struct is_dispatcher_contract : std::false_type {};

template <auto &ref>
requires is_ray_dispatcher_v <reference_base_of <ref>>
struct is_dispatcher_contract <contract <ref>> : std::true_type {};

template <typename T>
struct is_receiver_contract : std::false_type {};

template <auto &ref>
requires is_ray_receiver_v <reference_base_of <ref>>
struct is_receiver_contract <contract <ref>> : std::true_type {};

template <typename T>
struct contract_address;

template <auto &ref>
struct contract_address <contract <ref>> {
	static constexpr void *value = reference_base_of <ref> ::address;
};

template <void *Addr>
struct address_tag {};

template <typename T>
struct contract_to_address_tag {
	using type = address_tag <contract_address <T> ::value>;
};

template <typename Shader>
using dispatcher_addresses_t = tlist_unique_t <tlist_transform_t <
	tlist_filter_t <is_dispatcher_contract, typename Shader::args>,
	contract_to_address_tag
>>;

template <typename Shader>
using receiver_addresses_t = tlist_unique_t <tlist_transform_t <
	tlist_filter_t <is_receiver_contract, typename Shader::args>,
	contract_to_address_tag
>>;

template <typename Subset, typename Superset>
struct tlist_subset_of;

template <typename Superset>
struct tlist_subset_of <Tlist <>, Superset> : std::true_type {};

template <typename Head, typename ... Rest, typename Superset>
struct tlist_subset_of <Tlist <Head, Rest...>, Superset>
	: std::bool_constant <
		tlist_contains <Head, Superset> ::value
		and tlist_subset_of <Tlist <Rest...>, Superset> ::value
	> {};

template <typename ... Shaders>
using all_receiver_addresses_t = tlist_unique_t <tlist_concat_t <
	receiver_addresses_t <Shaders>...
>>;

template <typename ... Shaders>
using all_dispatcher_addresses_t = tlist_unique_t <tlist_concat_t <
	dispatcher_addresses_t <Shaders>...
>>;

// Raytracing: every receiver must have a matching dispatcher
template <typename DispatcherAddresses, typename ReceiverAddresses>
constexpr bool raytracing_receivers_covered =
	tlist_subset_of <ReceiverAddresses, DispatcherAddresses> ::value;

} // namespace rcgp
