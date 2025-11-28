#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <vector>
#include <glm/glm.hpp>
#include <catch2/catch_test_macros.hpp>

#include <ugp.hpp>

using PaddedVec3 = padded_t <glm::vec3, 4>;

TEST_CASE("std430 trivial tuple writes match contiguous struct layout", "[std430][layout]")
{
	using layout = std430_layout_t <uint32_t, glm::vec3>;

	struct Mirror {
		padded_t <uint32_t, 12> scalar;
		PaddedVec3 vector;
	};

	layout tuple {};
	tuple.template get <0> () = 0x11223344u;
	tuple.template get <1> () = glm::vec3(1.0f, 2.0f, 3.0f);

	Mirror expected {};
	expected.scalar = tuple.template get <0> ();
	expected.vector = tuple.template get <1> ();

	static_assert(sizeof(layout) == sizeof(Mirror));

	std::array <std::byte, sizeof(layout)> tuple_bytes {};
	std::array <std::byte, sizeof(Mirror)> mirror_bytes {};

	std::memcpy(tuple_bytes.data(), &tuple, tuple_bytes.size());
	std::memcpy(mirror_bytes.data(), &expected, mirror_bytes.size());

	REQUIRE(tuple_bytes == mirror_bytes);
}

TEST_CASE("std430 dynamic tuple writes match contiguous struct layout", "[std430][layout][dynamic]")
{
	using layout = std430_layout_t <uint32_t, glm::vec3[]>;

	struct Mirror {
		padded_t <uint32_t, 12> scalar;
		PaddedVec3 vectors[2];
	};

	layout tuple {};
	tuple.template get <0> () = 0xAABBCCDDu;
	tuple.dynamic().resize(2);
	tuple.dynamic()[0] = glm::vec3(1.0f, 2.0f, 3.0f);
	tuple.dynamic()[1] = glm::vec3(4.0f, 5.0f, 6.0f);

	Mirror expected {};
	expected.scalar = tuple.template get <0> ();
	expected.vectors[0] = tuple.dynamic()[0];
	expected.vectors[1] = tuple.dynamic()[1];

	REQUIRE(layout::size(tuple.dynamic().size()) == sizeof(Mirror));

	const auto mirror_vectors_offset = static_cast <size_t> (
		reinterpret_cast <std::byte *> (&expected.vectors)
		- reinterpret_cast <std::byte *> (&expected)
	);
	REQUIRE(layout::dynamic_offset() == mirror_vectors_offset);

	std::vector <std::byte> tuple_bytes(layout::size(tuple.dynamic().size()), std::byte { 0 });
	std::memcpy(tuple_bytes.data(), &tuple.statics(), sizeof(tuple.statics()));
	std::memcpy(tuple_bytes.data() + layout::dynamic_offset(), tuple.dynamic().data(), sizeof(PaddedVec3) * tuple.dynamic().size());

	std::array <std::byte, sizeof(Mirror)> mirror_bytes {};
	std::memcpy(mirror_bytes.data(), &expected, mirror_bytes.size());

	REQUIRE(std::equal(tuple_bytes.begin(), tuple_bytes.end(), mirror_bytes.begin(), mirror_bytes.end()));
}
