#include <type_traits>
#include <glm/glm.hpp>

#include <ugp.hpp>

using basic_static = std430_layout_t <uint32_t, glm::vec3[3], uint32_t>;
static_assert(sizeof(basic_static) == 80);
static_assert(basic_static::offset <0> () == 0);
static_assert(basic_static::offset <1> () == 16);
static_assert(basic_static::offset <2> () == 64);

// Static-only tuple with vec3 tail needing alignment
using vec3_tail_padding = std430_layout_t <uint32_t, glm::vec3>;
static_assert(std::is_same_v <
	vec3_tail_padding,
	trivial_tuple <
		padded_t <uint32_t, 12>,
		padded_t <glm::vec3, 4>
	>
>);
static_assert(vec3_tail_padding::offset <0> () == 0);
static_assert(vec3_tail_padding::offset <1> () == 16);
static_assert(sizeof(vec3_tail_padding) == 32);

// Multiple vec2 entries plus scalar to check carry alignment
using repeated_vec2 = std430_layout_t <glm::vec2, glm::vec2, uint32_t>;
static_assert(std::is_same_v <
	repeated_vec2,
	trivial_tuple <
		padded_t <glm::vec2, 0>,
		padded_t <glm::vec2, 0>,
		padded_t <uint32_t, 4>
	>
>);
static_assert(repeated_vec2::offset <0> () == 0);
static_assert(repeated_vec2::offset <1> () == 8);
static_assert(repeated_vec2::offset <2> () == 16);
static_assert(sizeof(repeated_vec2) == 24);

// Nested aggregate should respect inner alignment
using nested_struct = std430_layout_t <sequence <glm::vec3, uint32_t>, glm::vec2>;
static_assert(nested_struct::offset <0> () == 0);
static_assert(nested_struct::offset <1> () == 16);
static_assert(sizeof(nested_struct) == 32);

// Single nested aggregate should remain tightly packed
using single_nested = std430_layout_t <sequence <glm::vec2>>;
static_assert(single_nested::offset <0> () == 0);
static_assert(sizeof(single_nested) == 8);

// Static array followed by smaller vector
using array_and_vec = std430_layout_t <glm::vec3[2], glm::vec2>;
static_assert(std::is_same_v <
	array_and_vec,
	trivial_tuple <
		padded_t <padded_t <glm::vec3, 4> [2], 0>,
		padded_t <glm::vec2, 8>
	>
>);
static_assert(array_and_vec::offset <0> () == 0);
static_assert(array_and_vec::offset <1> () == 32);
static_assert(sizeof(array_and_vec) == 48);

// Dynamic vec3 array after a scalar
using dynamic_vec3 = std430_layout_t <uint32_t, glm::vec3[]>;
static_assert(std::is_same_v <
	dynamic_vec3,
	dynamic_tuple <
		padded_t <glm::vec3, 4> [],
		padded_t <uint32_t, 12>
	>
>);
static_assert(dynamic_vec3::offset <0> () == 0);
static_assert(dynamic_vec3::dynamic_offset() == 16);

// Mixed statics feeding into dynamic vec3 array
using mixed_static_dynamic = std430_layout_t <glm::vec3, glm::vec2, glm::vec3[]>;
static_assert(std::is_same_v <
	mixed_static_dynamic,
	dynamic_tuple <
		padded_t <glm::vec3, 4> [],
		padded_t <glm::vec3, 4>,
		padded_t <glm::vec2, 8>
	>
>);
static_assert(mixed_static_dynamic::offset <0> () == 0);
static_assert(mixed_static_dynamic::offset <1> () == 16);
static_assert(mixed_static_dynamic::dynamic_offset() == 32);
