#pragma once

#include "layouts.hpp"
#include "mirror_buffer.hpp"

namespace rcgp {

enum class Topology {
	eTriangleList,
	eLineList,
	eTriangleFan,
};

consteval vk::PrimitiveTopology translate_topology(Topology T)
{
	using enum vk::PrimitiveTopology;

	switch (T) {
	case Topology::eTriangleList: return eTriangleList;
	case Topology::eLineList: return eLineList;
	case Topology::eTriangleFan: return eTriangleFan;
	}

	return vk::PrimitiveTopology::eTriangleList;
}

template <Topology T, typename I>
using topology_element_t = std::conditional_t <
	T == Topology::eTriangleList,
	vector <I, 3>,
	std::conditional_t <
		T == Topology::eLineList,
		vector <I, 2>,
		scalar <I>
	>
>;

template <Topology T, typename I>
using IndexBuffer = IndexMirrorBuffer <
	array <topology_element_t <T, I>>,
	layouts::scalar
>;

} // namespace rcgp
