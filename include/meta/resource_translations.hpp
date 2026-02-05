#pragma once

#include "../util/cti.hpp"
#include "resources.hpp"
#include "static_string.hpp"
#include "contract.hpp"

namespace rcgp {

// Forward declarations
template <typename T, template <typename> typename L, vk::BufferUsageFlagBits F>
struct MirrorBuffer;

struct MirrorSampler;

template <typename T>
struct resource_translation {};

template <typename T>
using resource_translation_handle_t = resource_translation <T> ::handle_type;

template <typename T, template <typename> typename L, vk::VertexInputRate R>
struct resource_translation <AttributeStream <T, L, R>> {
	using handle_type = MirrorBuffer <array <T>, L, vk::BufferUsageFlagBits::eVertexBuffer>;
	using host_type = handle_type::value_type;
};

template <typename T, template <typename> typename L>
struct resource_translation <PushConstant <T, L>> {
	using hint = L <T> ::hint;
	using handle_type = scaffold_lookup <hint, T, true> ::type;
	using host_type = handle_type;
};

template <typename T, template <typename> typename L>
struct resource_translation <UniformBuffer <T, L>> {
	using handle_type = MirrorBuffer <T, L, vk::BufferUsageFlagBits::eUniformBuffer>;
	using host_type = handle_type::value_type;
};

template <typename T, template <typename> typename L, GlobalResourceAccess A>
struct resource_translation <StorageBuffer <T, L, A>> {
	using handle_type = MirrorBuffer <T, L, vk::BufferUsageFlagBits::eStorageBuffer>;
	using host_type = handle_type::value_type;
};

template <typename T, size_t D>
struct resource_translation <Sampler <T, D>> {
	using handle_type = MirrorSampler;
	using host_type = std::nullptr_t;
};

template <user_defined T>
struct resource_translation <ResourceGroup <T>> {
	using translated = T::fields::template map <resource_translation_handle_t>;
	using hints = translated::template map <scaffold_natural>;
	using handle_type = scaffold_lookup <scaffold_natural <hints>, T> ::type;
	using host_type = std::nullptr_t;
};

// namespace resource_layout {
//
// // TODO: laayout engine is a bad name... also need an alias for naturally
// // aligned scaffold hints
// template <typename T>
// struct layout_engine {
// 	static_error("resource_layout::layout_engine not implemented for type "_ss + $ss_type(T));
// };
//
// template <typename List>
// struct layout_engine_list;
//
// template <typename ... Ts>
// struct layout_engine_list <Tlist <Ts...>> {
// 	using hint = scaffold_natural <
// 		Tlist <typename layout_engine <Ts> ::hint...>
// 	>;
// };
//
// template <user_defined T>
// struct layout_engine <T> : layout_engine_list <typename T::fields> {};
//
// template <typename T, int64_t N>
// requires (N > 0)
// struct layout_engine <array <T, N>> {
// 	using hint = scaffold_natural <
// 		std::array <typename layout_engine <T> ::hint, N>
// 	>;
// };
//
// template <typename T>
// struct layout_engine <array <T, -1>> {
// 	using hint = scaffold_natural <
// 		unsized_array <typename layout_engine <T> ::hint>
// 	>;
// };
//
// template <typename T, template <typename> typename L>
// struct layout_engine <UniformBuffer <T, L>> {
// 	using hint = scaffold_hint <ResourceType <UniformBuffer <T, L>>, 0>;
// };
//
// template <typename T, template <typename> typename L, GlobalResourceAccess A>
// struct layout_engine <StorageBuffer <T, L, A>> {
// 	using hint = scaffold_hint <ResourceType <StorageBuffer <T, L, A>>, 0>;
// };
//
// template <typename T, size_t D>
// struct layout_engine <Sampler <T, D>> {
// 	using hint = scaffold_hint <ResourceType <Sampler <T, D>>, 0>;
// };
//
// template <typename T>
// struct layout_engine <ResourceGroup <T>> {
// 	using hint = typename layout_engine <T> ::hint;
// };
//
// } // namespace resource_layout

// template <typename T>
// using ResourceMirror = decltype([] {
// 	using hint = resource_layout::layout_engine <T> ::hint;
// 	using type = scaffold_lookup <hint, T, true> ::type;
// 	return type();
// } ());

// template <user_defined T>
// struct resource_translator <T> {
// 	using type = ResourceMirror <T>;
// 	using value_type = type;
// 	using element_type = std::nullptr_t;
// };
//
// template <typename T>
// struct resource_translator <ResourceGroup <T>> {
// 	using type = ResourceMirror <T>;
// 	using value_type = type;
// 	using element_type = std::nullptr_t;
// };
//
// template <typename T, template <typename> typename L, vk::VertexInputRate R>
// struct resource_translator <AttributeStream <T, L, R>> {
// 	using buffer = VertexMirrorBuffer <array <T>, L>;
// 	using type = buffer;
// 	using value_type = buffer::value_type;
// 	using element_type = buffer::element_type;
// };
//
// template <typename T, template <typename> typename L>
// struct resource_translator <PushConstant <T, L>> {
// 	using type = TypeMirror <T, L>;
// 	using value_type = type;
// 	using element_type = type;
// };
//
// template <native_scalar T, size_t D>
// struct resource_translator <Sampler <T, D>> {
// 	using type = SamplerMirror;
// 	using value_type = type;
// 	using element_type = std::nullptr_t;
// };

} // namespace rcgp
