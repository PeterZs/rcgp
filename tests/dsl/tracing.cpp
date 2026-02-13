#include "common.hpp"
#include "common_resources.hpp"

#define SUITE "tracing"

// Tracing:
// Testing that user-authored shader modules and subroutines are correctly
// trascribed into instructions. This primarily tests for argument injection,
// return handling, and subroutine invocation.
// TODO: verbose_tracing and normal tracing

add_test(vs_empty)
{
	auto vs = $shader(vertex)() {};

	assert_assembly_match(vs, R"(
	Block {
	  Context {
	    stage: Vertex,
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
	Block {
	  Context {
	    stage: Vertex,
	  }
	  $0 = Local Float
	  $1 = 1
	  Store $0 $1
	  $2 = New Vec4($0, $0, $0, $0)
	  $3 = Local Vec4
	  Store $3 $2
	  $4 = SV: ClipPosition
	  Store $4 $3
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
	Block {
	  Context {
	    stage: Vertex,
	    stage outputs: { Smooth Vec3, Flat UVec2 }
	  }
	  $0 = Local Float
	  $1 = 1
	  Store $0 $1
	  $2 = New Vec3($0, $0, $0)
	  $3 = Local Vec3
	  Store $3 $2
	  $4 = StageOutput 0: Smooth Vec3
	  Store $4 $3
	  $5 = Local UInt32
	  $6 = 4
	  Store $5 $6
	  $7 = Local UInt32
	  $8 = 1
	  Store $7 $8
	  $9 = New UVec2($7, $5)
	  $10 = Local UVec2
	  Store $10 $9
	  $11 = StageOutput 1: Flat UVec2
	  Store $11 $10
	}
	)");
};

add_test(vs_stream)
{
	auto vs = $shader(vertex)(
		$contracts((p, fwd::position)),
		ClipPosition cpos
	) -> float3
	{
		cpos = float4(p, 1);
		return p;
	};

	assert_assembly_match(vs, R"(
	Block {
	  Context {
	    stage: Vertex,
	    stage inputs: { Smooth Vec3 }
	    stage outputs: { Smooth Vec3 }
	  }
	  $0 = StageInput 0: Smooth Vec3
	  $1 = Local Float
	  $2 = 1
	  Store $1 $2
	  $3 = New Vec4($0, $1)
	  $4 = Local Vec4
	  Store $4 $3
	  $5 = SV: ClipPosition
	  Store $5 $4
	  $6 = StageOutput 0: Smooth Vec3
	  Store $6 $0
	}
	)");
};

add_test(vs_multiple_io)
{
	auto vs = $shader(vertex)(
		$contracts(
			(pos, fwd::position),
			(nrm, fwd::normal),
			(uv, fwd::uv)
		)
	) {
		return std::tuple {
			Smooth <float3> { pos },
			Smooth <float3> { nrm },
			Smooth <float2> { uv },
		};
	};
	
	assert_assembly_match(vs, R"(
	Block {
	  Context {
	    stage: Vertex,
	    stage inputs: { Smooth Vec3, Smooth Vec3, Smooth Vec2 }
	    stage outputs: { Smooth Vec3, Smooth Vec3, Smooth Vec2 }
	  }
	  $0 = StageInput 0: Smooth Vec3
	  $1 = StageInput 1: Smooth Vec3
	  $2 = StageInput 2: Smooth Vec2
	  $3 = StageOutput 0: Smooth Vec3
	  Store $3 $0
	  $4 = StageOutput 1: Smooth Vec3
	  Store $4 $1
	  $5 = StageOutput 2: Smooth Vec2
	  Store $5 $2
	}
	)");
};

add_test(vs_push_constant)
{
	auto vs = $shader(vertex)(
		$contracts(
			(view, fwd::view),
			(pos, fwd::position)
		),
		ClipPosition cpos
	) -> float3
	{
		float4 wpos = view.model * float4(pos, 1);
		cpos = view.proj * view.view * wpos;
		return float3(wpos);
	};

	assert_assembly_match(vs, R"(
	Block {
	  Context {
	    stage: Vertex,
	    stage inputs: { Smooth Vec3 }
	    stage outputs: { Smooth Vec3 }
	    resources: { $0 },
	  }
	  $0 = PushConstant +4294967295: Std430 fwd::View
	  $1 = $0.model
	  $2 = $0.view
	  $3 = $0.proj
	  $4 = StageInput 0: Smooth Vec3
	  $5 = Local Float
	  $6 = 1
	  Store $5 $6
	  $7 = New Vec4($4, $5)
	  $8 = Local Vec4
	  Store $8 $7
	  $9 = Multiply $1 $8
	  $10 = Local Vec4
	  Store $10 $9
	  $11 = Multiply $3 $2
	  $12 = Local FMat4x4
	  Store $12 $11
	  $13 = Multiply $12 $10
	  $14 = Local Vec4
	  Store $14 $13
	  $15 = SV: ClipPosition
	  Store $15 $14
	  $16 = New Vec3($10)
	  $17 = Local Vec3
	  Store $17 $16
	  $18 = StageOutput 0: Smooth Vec3
	  Store $18 $17
	}
	)");
};

add_test(sr_return_primitives)
{
	auto sr = $subroutine(sr)(f32 x, u32 y) {
		return std::tuple { float3(x), uint2(y, 13) };
	};
	
	assert_assembly_match(sr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: sr,
	    arguments: { Float, UInt32 },
	    returns: { Vec3, UVec2 },
	  }
	  $0 = Argument 0: Float
	  $1 = Argument 1: UInt32
	  $2 = New Vec3($0, $0, $0)
	  $3 = Local Vec3
	  Store $3 $2
	  $4 = Local UInt32
	  $5 = 13
	  Store $4 $5
	  $6 = New UVec2($1, $4)
	  $7 = Local UVec2
	  Store $7 $6
	  $9 = Return 0: $8
	  Store $9 $3
	  $11 = Return 1: $10
	  Store $11 $7
	}
	)");
};

add_test(sr_return_aggregate)
{
	auto sr = $subroutine(sr)(f32 z) {
		return Ray {
			float3(0),
			normalize(float3(1, z, 1)),
		};
	};
	
	assert_assembly_match(sr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: sr,
	    arguments: { Float },
	    returns: { Ray },
	  }
	  $0 = Argument 0: Float
	  $1 = Local Float
	  $2 = 0
	  Store $1 $2
	  $3 = New Vec3($1, $1, $1)
	  $4 = Local Vec3
	  Store $4 $3
	  $5 = Local Float
	  $6 = 1
	  Store $5 $6
	  $7 = Local Float
	  $8 = 1
	  Store $7 $8
	  $9 = New Vec3($7, $0, $5)
	  $10 = Local Vec3
	  Store $10 $9
	  $11 = Normalize($10)
	  $12 = Local Vec3
	  Store $12 $11
	  $14 = Return 0: $13
	  $15 = New Ray($4, $12)
	  Store $14 $15
	}
	)");
};

add_test(sr_invocation)
{
	auto sr1 = $subroutine(sr1)(f32 x) {
		return float3(x);
	};
	
	auto sr2 = $subroutine(sr2)(f32 x, u32 y) {
		return std::tuple { float3(x), uint2(y, 13) };
	};
	
	auto sr3 = $subroutine(sr3)(f32 z) {
		return Ray {
			float3(0),
			normalize(float3(1, z, 1)),
		};
	};

	auto vs = $shader(vertex, &)() {
		auto r1 = $use(sr1)(1);
		auto [r2a, r2b] = $use(sr2)(1.0f, 2);
		auto r3 = $use(sr3)(2.0f);
		return std::tuple { r1, r2a, r2b, r3.origin, r3.direction };
	};

	assert_assembly_match(vs, R"(
	Block {
	  Context {
	    stage: Vertex,
	    stage outputs: { Smooth Vec3, Smooth Vec3, Smooth UVec2, Smooth Vec3, Smooth Vec3 }
	  }
	  $0 = Local Float
	  $1 = 1
	  Store $0 $1
	  $2 = Local Vec3
	  sr1($0, $2)
	  $3 = Local UInt32
	  $4 = 2
	  Store $3 $4
	  $5 = Local Float
	  $6 = 1
	  Store $5 $6
	  $7 = Local Vec3
	  $8 = Local UVec2
	  sr2($5, $3, $7, $8)
	  $9 = Local Float
	  $10 = 2
	  Store $9 $10
	  $11 = Local Ray
	  sr3($9, $11)
	  $12 = $11.origin
	  $13 = $11.direction
	  $14 = StageOutput 0: Smooth Vec3
	  Store $14 $2
	  $15 = StageOutput 1: Smooth Vec3
	  Store $15 $8
	  $16 = StageOutput 2: Smooth UVec2
	  Store $16 $7
	  $17 = StageOutput 3: Smooth Vec3
	  Store $17 $12
	  $18 = StageOutput 4: Smooth Vec3
	  Store $18 $13
	}
	)");
};
