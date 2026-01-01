#include "dsl/instructions.hpp"

std::string swizzle_string(Swizzle::Code code)
{
	static constexpr char letters[] = "xyzw";

	auto raw = static_cast <int> (code);
	if (raw < 0)
		return "?";

	int length = 1;
	int offset = 0;
	int count = 4;
	while (length <= 4 && raw >= offset + count) {
		offset += count;
		count *= 4;
		length++;
	}

	if (length > 4 || raw < offset)
		return "?";

	int index = raw - offset;
	int divisor = 1;
	for (int i = 1; i < length; i++)
		divisor *= 4;

	std::string result;
	result.reserve(length);
	for (int i = 0; i < length; i++) {
		int digit = index / divisor;
		index %= divisor;
		result.push_back(letters[digit]);
		divisor /= 4;
	}

	return result;
}

void Block::apply_group_allocation_map(const group_allocation_map &map)
{
	for (auto &[addr, group] : map) {
		auto &refs = context.global_resources[addr];
		for (auto &ref : refs)
			ref->as <GlobalResource> ().group = group;
	}
}
