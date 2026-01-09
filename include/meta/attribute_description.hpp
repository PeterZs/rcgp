#pragma once

#include "reference.hpp"
#include "resources.hpp"
#include "symbolic_format.hpp"

template <auto &... refs>
constexpr auto sequence_to_vertex_attributes(const Tlist <reference <refs>...> &in)
{
	auto desc = [] <typename T, template <typename> typename L, vk::VertexInputRate R>
	(const AttributeStream <T, L, R> &) {
		return vk::VertexInputAttributeDescription()
			.setFormat(symbolic_format <T> ::value)
			.setOffset(0);
	};

	if constexpr (sizeof...(refs) == 0) {
		return std::array <vk::VertexInputAttributeDescription, 0> ();
	} else {
		size_t location = 0;
		return cti_constexpr_for(Is, sizeof...(refs),
			return std::array {
				desc(refs)
					.setBinding(Is)
					.setLocation(location++)...
			}
		);
	}
}
