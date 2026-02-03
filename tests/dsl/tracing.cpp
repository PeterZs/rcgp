#include "common.hpp"

#define SUITE "tracing"

// Resources
// TODO: shared header...
struct Ray {
	float3 origin;
	float3 direction;

	$reflection(origin, direction);
};

struct View {
	float4x4 model;
	float4x4 view;
	float4x4 proj;

	$reflection(model, view, proj);
};

static PushConstant <View> view;

static AttributeStream <float3> position;

// Tests
add_test(vs_empty)
{
	auto vs = $shader(vertex)() {};

	assert_assembly_match(vs, R"(
	block {
	  context {
	    model: vertex shader,
	    name: main,
	  }
	}
	)");
};

add_test(vs_clip)
{
	auto vs = $shader(vertex)(ClipPosition clip)
	{
		clip = float4(1);
	};

	assert_assembly_match(vs, R"(
	block {
	  context {
	    model: vertex shader,
	    name: main,
	  }
	  $0 = Float
	  $1 = local $0
	  $2 = 1
	  store $1 $2
	  $3 = Vec4
	  $4 = new $3($1, $1, $1, $1)
	  $5 = local $3
	  store $5 $4
	  $6 = SVPosition
	  store $6 $5
	}
	)");
};

add_test(vs_louts)
{
	auto vs = $shader(vertex)()
	{
		return std::tuple {
			Smooth { float3(1.0) },
			Flat { uint2(1, 4) },
		};
	};

	assert_assembly_match(vs, R"(
	block {
	  context {
	    model: vertex shader,
	    name: main,
	    stage out 0: $0 (smooth),
	    stage out 1: $1 (flat),
	  }
	  $2 = Float
	  $3 = local $2
	  $4 = 1
	  store $3 $4
	  $0 = Vec3
	  $5 = new $0($3, $3, $3)
	  $6 = local $0
	  store $6 $5
	  $7 = stage out($0, 0, smooth)
	  store $7 $6
	  $8 = UInt32
	  $9 = local $8
	  $10 = 4
	  store $9 $10
	  $11 = local $8
	  $12 = 1
	  store $11 $12
	  $1 = UVec2
	  $13 = new $1($11, $9)
	  $14 = local $1
	  store $14 $13
	  $15 = stage out($1, 1, flat)
	  store $15 $14
	}
	)");
};

add_test(vs_stream)
{
	auto vs = $shader(vertex)(
		$contracts(position),
		ClipPosition cpos
	) -> float3
	{
		cpos = float4(position, 1);
		return position;
	};

	assert_assembly_match(vs, R"(
	block {
	  context {
	    model: vertex shader,
	    name: main,
	    stage in 0: $0,
	    stage out 0: $0 (smooth),
	  }
	  $1 = Vec4
	  $2 = Float
	  $0 = Vec3
	  $3 = local $0
	  $4 = stage in($0, 0)
	  $5 = local $2
	  $6 = 1
	  store $5 $6
	  $7 = new $1($4, $5)
	  $8 = local $1
	  store $8 $7
	  $9 = SVPosition
	  store $9 $8
	  $10 = stage out($0, 0, smooth)
	  store $10 $4
	}
	)");
};

add_test(vs_push_constant)
{
	auto vs = $shader(vertex)(
		$contracts(view, position),
		ClipPosition cpos
	) -> float3
	{
		float4 wpos = view.model * float4(position, 1);
		cpos = view.proj * view.view * wpos;
		return float3(wpos);
	};

	assert_assembly_match(vs, R"(
	block {
	  context {
	    model: vertex shader,
	    name: main,
	    stage in 0: $0,
	    stage out 0: $0 (smooth),
	    resource: {$1},
	  }
	  $2 = Vec4
	  $3 = Float
	  $0 = Vec3
	  $4 = local $0
	  $5 = FMat4x4
	  $6 = local $5
	  $7 = local $5
	  $8 = local $5
	  $9 = local $5
	  $10 = local $5
	  $11 = local $5
	  $12 = View($5, $5, $5)
	  $1 = push_constant($12, nil:0, Std430)
	  $13 = field $1:0
	  $14 = field $1:1
	  $15 = field $1:2
	  $16 = stage in($0, 0)
	  $17 = local $3
	  $18 = 1
	  store $17 $18
	  $19 = new $2($16, $17)
	  $20 = local $2
	  store $20 $19
	  $21 = mul($13, $20)
	  $22 = mul($15, $14)
	  $23 = mul($22, $21)
	  $24 = SVPosition
	  store $24 $23
	  $25 = new $0($21)
	  $26 = local $0
	  store $26 $25
	  $27 = stage out($0, 0, smooth)
	  store $27 $26
	}
	)");
};

add_test(sr_return_primitives)
{
	auto sr = $subroutine(sr)() {
		return std::tuple { float3(1), uint2(12, 13) };
	};
	
	assert_assembly_match(sr, R"(
	block {
	  context {
	    model: subroutine,
	    name: sr,
	    return 0: $0,
	    return 1: $1,
	  }
	  $1 = UVec2
	  $2 = UInt32
	  $0 = Vec3
	  $3 = Float
	  $4 = local $3
	  $5 = 1
	  store $4 $5
	  $6 = new $0($4, $4, $4)
	  $7 = local $0
	  store $7 $6
	  $8 = local $2
	  $9 = 13
	  store $8 $9
	  $10 = local $2
	  $11 = 12
	  store $10 $11
	  $12 = new $1($10, $8)
	  $13 = local $1
	  store $13 $12
	  $14 = return($0, 0)
	  store $14 $7
	  $15 = return($1, 1)
	  store $15 $13
	}
	)");
};

add_test(sr_return_aggregate)
{
	auto sr = $subroutine(sr)() {
		return Ray {
			float3(0),
			normalize(float3(1, 1, 1)),
		};
	};
	
	assert_assembly_match(sr, R"(
	block {
	  context {
	    model: subroutine,
	    name: sr,
	    return 0: $0,
	  }
	  $1 = Vec3
	  $2 = Float
	  $3 = local $2
	  $4 = 0
	  store $3 $4
	  $5 = new $1($3, $3, $3)
	  $6 = local $1
	  store $6 $5
	  $7 = local $2
	  $8 = 1
	  store $7 $8
	  $9 = local $2
	  $10 = 1
	  store $9 $10
	  $11 = local $2
	  $12 = 1
	  store $11 $12
	  $13 = new $1($11, $9, $7)
	  $14 = local $1
	  store $14 $13
	  $15 = normalize($14)
	  $0 = Ray($1, $1)
	  $16 = return($0, 0)
	  $17 = new $0($6, $15)
	  store $16 $17
	}
	)");
};

// TODO: subroutine invocation
