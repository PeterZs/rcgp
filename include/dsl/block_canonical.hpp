#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "instruction_nodes.hpp"

namespace rcgp {

struct BlockCanonical {
	std::array <uint64_t, 2> hash;
	std::vector <std::byte> bytes;
};

BlockCanonical canonicalize_block(const SharedBlockReference &sbr);

std::array <uint64_t, 2> fnv1a_128(std::span <const std::byte>);
std::array <uint64_t, 2> fnv1a_128(std::string_view);

} // namespace rcgp
