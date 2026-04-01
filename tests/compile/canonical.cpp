#include <rcgp.hpp>

using namespace rcgp;

// --- canonical_type mappings ---

// Native scalars canonicalize to DSL scalars
static_assert(std::same_as <canonical_type_t <int32_t>, scalar <int32_t>>);
static_assert(std::same_as <canonical_type_t <uint32_t>, scalar <uint32_t>>);
static_assert(std::same_as <canonical_type_t <float>, scalar <float>>);
static_assert(std::same_as <canonical_type_t <bool>, scalar <bool>>);

// C++ arithmetic types canonicalize to DSL scalars
static_assert(std::same_as <canonical_type_t <int>, scalar <int32_t>>);
static_assert(std::same_as <canonical_type_t <unsigned int>, scalar <uint32_t>>);
static_assert(std::same_as <canonical_type_t <double>, scalar <float>>);

// DSL scalars are identity
static_assert(std::same_as <canonical_type_t <i32>, i32>);
static_assert(std::same_as <canonical_type_t <f32>, f32>);
static_assert(std::same_as <canonical_type_t <u32>, u32>);
static_assert(std::same_as <canonical_type_t <boolean>, boolean>);

// Vectors are identity
static_assert(std::same_as <canonical_type_t <float3>, float3>);
static_assert(std::same_as <canonical_type_t <int3>, int3>);

// --- Concept checks ---

static_assert(canonically_scalar <int32_t>);
static_assert(canonically_scalar <float>);
static_assert(canonically_scalar <int>);
static_assert(canonically_scalar <double>);
static_assert(canonically_scalar <i32>);
static_assert(canonically_scalar <f32>);
static_assert(not canonically_scalar <float3>);
static_assert(not canonically_scalar <mat4>);

static_assert(canonically_integral <int32_t>);
static_assert(canonically_integral <uint32_t>);
static_assert(canonically_integral <int>);
static_assert(canonically_integral <unsigned int>);
static_assert(canonically_integral <i32>);
static_assert(canonically_integral <u32>);
static_assert(not canonically_integral <float>);
static_assert(not canonically_integral <f32>);
static_assert(not canonically_integral <double>);

static_assert(canonically_equivalent <int32_t, i32>);
static_assert(canonically_equivalent <float, f32>);
static_assert(canonically_equivalent <int, i32>);
static_assert(canonically_equivalent <double, f32>);
static_assert(not canonically_equivalent <i32, f32>);
static_assert(not canonically_equivalent <int, float>);

// --- Promotion checks ---

static_assert(std::same_as <promoted_scalar_t <i32, i32>, i32>);
static_assert(std::same_as <promoted_scalar_t <f32, f32>, f32>);
static_assert(std::same_as <promoted_scalar_t <i32, f32>, f32>);
static_assert(std::same_as <promoted_scalar_t <f32, i32>, f32>);
static_assert(std::same_as <promoted_scalar_t <u32, f32>, f32>);

// promotable_scalars requires at least one DSL type
static_assert(promotable_scalars <i32, f32>);
static_assert(promotable_scalars <f32, int32_t>);
static_assert(promotable_scalars <int, f32>);
static_assert(not promotable_scalars <int, float>);  // both native

// --- Runtime expression compilation checks ---
// These lambdas verify that cross-type arithmetic, vector-scalar ops,
// and comparisons with native types compile correctly through the
// canonical/promotion system. They are not executed — compilation is
// the test.

auto same_type_scalar_add = [] {
	i32 a = 1;
	i32 b = 2;
	i32 c = a + b;
};

auto cross_type_scalar_add = [] {
	f32 a = 1.0f;
	i32 b = 2;
	f32 c = a + b;
};

auto native_scalar_with_dsl = [] {
	i32 a = 5;
	i32 b = a + 3;
};

auto vector_scalar_multiply = [] {
	float3 v(1.0f, 2.0f, 3.0f);
	float3 w = 2.0f * v;
};

auto vector_native_int_multiply = [] {
	float3 v(1.0f, 2.0f, 3.0f);
	float3 w = 2 * v;
};

auto comparison_with_native = [] {
	i32 a = 5;
	boolean b = a < 10;
};
