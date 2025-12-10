#pragma once

#include "reflection.hpp"
#include "../dsl/primitives.hpp"

template <reflected T>
struct symbolic_format {
	static_assert(false, "bad");
};

template <>
struct symbolic_format <vec2> : std::integral_constant <vk::Format, vk::Format::eR32G32Sfloat> {};

template <>
struct symbolic_format <vec3> : std::integral_constant <vk::Format, vk::Format::eR32G32B32Sfloat> {};

template <>
struct symbolic_format <vec4> : std::integral_constant <vk::Format, vk::Format::eR32G32B32A32Sfloat> {};
