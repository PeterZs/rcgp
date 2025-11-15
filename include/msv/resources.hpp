#pragma once

#include "reflection.hpp"
#include "expand_reflection.hpp"

// TODO: needs a layout...
template <typename T>
struct ParameterBlock {
	using reflection = parameter_block_reflection <T>;

	static constexpr bool _ugp_has_reflection = true;
};

template <typename T>
struct RayPayload : T {};
