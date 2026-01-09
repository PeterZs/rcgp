#pragma once

#include "mirror.hpp"
#include "reference.hpp"
#include "resources.hpp"

template <auto &... refs>
constexpr auto sequence_to_vertex_bindings(const Tlist <reference <refs>...> &in)
{
	auto desc = [] <typename T, template <typename> typename L, vk::VertexInputRate R>
	(const AttributeStream <T, L, R> &) {
		using M = TypeMirror <T, L>;
		return vk::VertexInputBindingDescription()
			.setStride(sizeof(M))
			.setInputRate(R);
	};

	if constexpr (sizeof...(refs) == 0) {
		return std::array <vk::VertexInputBindingDescription, 0> ();
	} else {
		return cti_constexpr_for(Is, sizeof...(refs),
			return std::array {
				desc(refs).setBinding(Is)...
			}
		);
	}
}
