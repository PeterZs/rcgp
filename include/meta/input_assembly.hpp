#pragma once

#include "mirror.hpp"
#include "reference.hpp"
#include "resources.hpp"
#include "symbolic_format.hpp"

namespace rcgp {

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
		return constexpr_for(Is, sizeof...(refs),
			return std::array {
				desc(refs).setBinding(Is)...
			}
		);
	}
}

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
		return constexpr_for(Is, sizeof...(refs),
			return std::array {
				desc(refs)
					.setBinding(Is)
					.setLocation(location++)...
			}
		);
	}
}

} // namespace rcgp
