#pragma once

#include "layout/all.hpp"
#include "mirror_buffer.hpp"

enum class Topology {
	eTriangleList,
	eTriangleFan,
};

// Index buffer specialization
template <Topology T, typename I>
using topology_element_t = std::conditional_t <
	T == Topology::eTriangleList,
	vector <I, 3>,
	scalar <I>
>;

template <Topology T, typename I>
using index_buffer_base_t = IndexMirrorBuffer <
	array <topology_element_t <T, I>>,
	layouts::scalar
>;

template <Topology T, typename I>
struct IndexBuffer : index_buffer_base_t <T, I> {
	using base = index_buffer_base_t <T, I>;

	IndexBuffer() = default;
	IndexBuffer(const base &other) : base(other) {}

	auto &write(const base::value_type &data) const {
		base::write(data);
		return *this;
	}

	template <typename U>
	auto &write_unsafe(std::span <U> memory, size_t offset = 0) const {
		Buffer::write <U> (memory, offset);
		return *this;
	}
	
	static IndexBuffer from(const Device &device,
			  	size_t max_elements,
			  	vk::MemoryPropertyFlags properties,
			  	vk::BufferUsageFlags extra_usage = vk::BufferUsageFlagBits(0)) {
		return base::from(device, max_elements, properties, extra_usage);
	}
};

// Translator for index buffers
template <Topology T, typename I>
struct resource_translator <IndexBuffer <T, I>> {
	using type = IndexBuffer <T, I>;
	using value_type = typename type::value_type;
	using element_type = typename type::element_type;
};
