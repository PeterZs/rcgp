#include "common.hpp"

#define SUITE "resolves"

add_test(vs_fs_io_rates)
{
	auto vs = $shader(vertex)() {
		return std::tuple {
			Smooth <f32> { 1 },
			NoPerspective <float2> { float2(1) },
			Flat <uint4> { uint4(1) },
		};
	};
	
	auto fs = $shader(fragment)(f32 x, float2 y, uint4 z) {
		return float3(1);
	};

	transfer_io_rates(vs, fs);

	assert_glsl_eq(fs, "resolves/vs_fs_io_rates.glsl");
};
