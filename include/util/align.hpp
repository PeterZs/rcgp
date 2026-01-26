#pragma once

#include <cstddef>

namespace rcgp {

constexpr size_t align_up(size_t value, size_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

} // namespace rcgp
