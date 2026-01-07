#pragma once

#include <type_traits>

#include "../dsl/jems.hpp"
#include "layout/all.hpp"
#include "reflection.hpp"
#include "reconstruct_type.hpp"

// Layout detection
template <template <typename> typename L>
consteval GlobalResourceLayout layout_of()
{
	using sample = primitive_reflection <scalar <int>>;
	if constexpr (std::is_same_v <L <sample>, layouts::std430 <sample>>)
		return GlobalResourceLayout::eStd430;
	else if constexpr (std::is_same_v <L <sample>, layouts::scalar <sample>>)
		return GlobalResourceLayout::eScalar;
	else
		static_assert(false, "unsupported layout for global resource");
}

// Implementations for intrinsic()
template <typename T, template <typename> typename L>
jems::handle push_constant_reflection <T, L> ::intrinsic(uint32_t binding)
{
	return jems::global_resource(
		reconstruct_type <T> (),
		GlobalResourceKind::ePushConstant,
		layout_of <L> (),
		std::nullopt,
		binding
	);
}

template <typename T, template <typename> typename L>
jems::handle uniform_buffer_reflection <T, L> ::intrinsic(uint32_t binding)
{
	return jems::global_resource(
		reconstruct_type <T> (),
		GlobalResourceKind::eUniformBuffer,
		layout_of <L> (),
		std::nullopt,
		binding
	);
}

template <typename T, template <typename> typename L>
jems::handle storage_buffer_reflection <T, L> ::intrinsic(uint32_t binding)
{
	return jems::global_resource(
		reconstruct_type <T> (),
		GlobalResourceKind::eStorageBuffer,
		layout_of <L> (),
		std::nullopt,
		binding
	);
}

template <typename T, size_t D>
jems::handle sampler_reflection <T, D> ::intrinsic(uint32_t binding)
{
	return jems::global_resource(
		jems::type(VectorType <T, D> ()),
		GlobalResourceKind::eSampler,
		GlobalResourceLayout::eNone,
		std::nullopt,
		binding
	);
}
