#pragma once

// Forward declarations
namespace vk { enum class VertexInputRate; }
namespace jems { class handle; }

#include "../dsl/instruction_enums.hpp"

// Reflection types
template <typename T>
struct primitive_reflection {};

// TODO: we only NEED reflection info for aggregates...
// everything else can be inferred from the top level type
template <typename Original, typename ... Ts>
struct aggregate_reflection {
	using original_type = Original;

	template <size_t I>
	using field_type = Ts...[I];
	
	static constexpr size_t field_count = sizeof...(Ts);

	// TODO: static_assert check here to ensure at most
	// one dynamic component (and finding the conflicting
	// ones)
};

template <typename T, int64_t N>
struct array_reflection {};

template <auto &ref, typename T>
struct reference_reflection {
	using value_type = T;
};

template <typename R, typename ... Args>
struct function_reflection {};

template <typename T>
struct resource_group_reflection {
	using value_type = T;
};

// TODO: move resource reflections to another file?
template <typename T, template <typename> typename L>
struct push_constant_reflection {
	static jems::handle intrinsic(uint32_t binding);
};

template <typename T, template <typename> typename L>
struct uniform_buffer_reflection {
	static jems::handle intrinsic(uint32_t binding);
};

template <typename T, template <typename> typename L, GlobalResourceAccess A>
struct storage_buffer_reflection {
	static jems::handle intrinsic(uint32_t binding);
};

template <typename T, size_t D>
struct sampler_reflection {
	static jems::handle intrinsic(uint32_t binding);
};

template <typename T, vk::VertexInputRate R>
struct attribute_stream_reflection {
	using value_type = T;
};

// Specific kinds of reflection
// TODO: replace with type traits
template <typename T>
struct is_primitive_reflection : std::false_type {};

template <typename U>
struct is_primitive_reflection <primitive_reflection <U>> : std::true_type {};

template <typename T>
constexpr bool is_primitive_reflection_v = is_primitive_reflection <T> ::value;

template <typename T>
struct is_aggregate_reflection : std::false_type {};

template <typename Original, typename ... Ts>
struct is_aggregate_reflection <aggregate_reflection <Original, Ts...>> : std::true_type {};

template <typename T>
constexpr bool is_aggregate_reflection_v = is_aggregate_reflection <T> ::value;

template <typename T>
struct is_dynamic_reflection : std::false_type {};

template <typename T>
struct is_dynamic_reflection <array_reflection <T, -1>> : std::true_type {};

template <typename Original, typename ... Ts>
struct is_dynamic_reflection <aggregate_reflection <Original, Ts...>>
	: std::bool_constant <(is_dynamic_reflection <Ts> ::value || ...)> {};

template <typename T>
constexpr bool is_dynamic_reflection_v = is_dynamic_reflection <T> ::value;

template <typename T>
constexpr bool is_static_reflection_v = not is_dynamic_reflection <T> ::value;

// Querying reflection status
template <typename T>
constexpr bool has_reflection()
{
	return requires { T::_rcgp_has_reflection; };
}

template <typename T>
constexpr bool has_aggregate_reflection()
{
	if constexpr (has_reflection <T> ())
		return is_aggregate_reflection_v <typename T::reflection>;
	else
		return false;
}

template <typename T>
constexpr bool has_primitive_reflection()
{
	if constexpr (has_reflection <T> ())
		return is_primitive_reflection_v <typename T::reflection>;
	else
		return false;
}

template <typename T>
concept reflected = has_reflection <T> ();

template <typename T>
concept aggregate = has_aggregate_reflection <T> ();

template <typename T>
concept primitive = has_primitive_reflection <T> ();
