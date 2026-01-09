#pragma once

#include "mirror.hpp"
#include "reference.hpp"
#include "reflection.hpp"
#include "resources.hpp"

template <reflected T, template <typename> typename L, vk::VertexInputRate R>
constexpr auto binding_description_for_attribute_stream(const AttributeStream <T, L, R> &, size_t binding)
{
	using M = TypeMirror <T, L>;
	return vk::VertexInputBindingDescription()
		.setBinding(binding)
		.setStride(sizeof(M))
		.setInputRate(R);
}

template <auto &... refs, size_t ... Is>
constexpr auto sequence_to_vertex_bindings_impl(const Tlist <reference <refs>...> &, const std::index_sequence <Is...> &)
{
	return std::array {
		binding_description_for_attribute_stream(refs, Is)...
	};
}

template <auto &... refs>
constexpr auto sequence_to_vertex_bindings(const Tlist <reference <refs>...> &in)
{
	if constexpr (sizeof...(refs) == 0)
		return std::array <vk::VertexInputBindingDescription, 0> ();
	else
		return sequence_to_vertex_bindings_impl(in, std::make_index_sequence <sizeof...(refs)> ());
}
