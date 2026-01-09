#pragma once

#include "../dsl/instruction_block.hpp"
#include "../util/tlist.hpp"

// Descriptor group allocation
template <auto &ref, size_t I>
struct group_allocation_record {
	static constexpr void *vptr = &ref;
	static constexpr size_t index = I;
};

template <typename ... Records>
group_allocation_map new_group_allocation_map(Tlist <Records...> records)
{
	return {
		{ Records::vptr, Records::index }...
	};
}
