#pragma once

#include "mirror_buffer.hpp"
#include "static_string.hpp"
#include "../util/cti.hpp"

// Reference to vertex buffer
template <auto &ref>
struct VertexBufferFor {
	static_error("not a stream"_ss);
};

template <reflected T, template <typename> typename L, vk::VertexInputRate R, AttributeStream <T, L, R> &ref>
struct VertexBufferFor <ref> : VertexMirrorBuffer <array <T>, L> {
	using super = VertexMirrorBuffer <array <T>, L>;

	VertexBufferFor() = default;
	VertexBufferFor(const super &other) : super(other) {}

	auto &write(const typename super::value_type &data) const {
		super::write(data);
		return *this;
	}

	template <typename U>
	auto &write_unsafe(std::span <const U> memory, size_t offset = 0) const {
		super::write_unsafe(memory, offset);
		return *this;
	}

	static VertexBufferFor from(
		const Device &device,
		size_t max_elements,
		vk::MemoryPropertyFlags properties,
		vk::BufferUsageFlags extra_usage = vk::BufferUsageFlagBits(0)
	) {
		return super::from(device, max_elements, properties, extra_usage);
	}
};
