#include "common.hpp"
#include "common_resources.hpp"

#define SUITE "tracing"

// Tracing:
// Testing that user-authored shader modules and subroutines are correctly
// trascribed into instructions. This primarily tests for argument injection,
// return handling, and subroutine invocation.

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
	  $5 = New Vec3($3)
	  $6 = Local Vec3
	  Store $6 $5
	  Store $4 $6
	  $7 = Local UInt32
	  $8 = 4
	  Store $7 $8
	  $9 = Local UInt32
	  $10 = 1
	  Store $9 $10
	  $11 = New UVec2($9, $7)
	  $12 = Local UVec2
	  Store $12 $11
	  $13 = StageOutput 1: Flat UVec2
	  $14 = New UVec2($12)
	  $15 = Local UVec2
	  Store $15 $14
	  Store $13 $15
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
	  $1 = New Vec3($0)
	  $2 = Local Vec3
	  Store $2 $1
	  $3 = Local Float
	  $4 = 1
	  Store $3 $4
	  $5 = New Vec4($2, $3)
	  $6 = Local Vec4
	  Store $6 $5
	  $7 = SV: ClipPosition
	  Store $7 $6
	  $8 = New Vec3($2)
	  $9 = Local Vec3
	  Store $9 $8
	  $10 = StageOutput 0: Smooth Vec3
	  Store $10 $9
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
	  $3 = New Vec2($2)
	  $4 = Local Vec2
	  Store $4 $3
	  $5 = New Vec3($1)
	  $6 = Local Vec3
	  Store $6 $5
	  $7 = New Vec3($0)
	  $8 = Local Vec3
	  Store $8 $7
	  $9 = StageOutput 0: Smooth Vec3
	  $10 = New Vec3($8)
	  $11 = Local Vec3
	  Store $11 $10
	  Store $9 $11
	  $12 = StageOutput 1: Smooth Vec3
	  $13 = New Vec3($6)
	  $14 = Local Vec3
	  Store $14 $13
	  Store $12 $14
	  $15 = StageOutput 2: Smooth Vec2
	  $16 = New Vec2($4)
	  $17 = Local Vec2
	  Store $17 $16
	  Store $15 $17
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
	  $5 = New Vec3($4)
	  $6 = Local Vec3
	  Store $6 $5
	  $7 = Local Float
	  $8 = 1
	  Store $7 $8
	  $9 = New Vec4($6, $7)
	  $10 = Local Vec4
	  Store $10 $9
	  $11 = Multiply $1 $10
	  $12 = Local Vec4
	  Store $12 $11
	  $13 = Multiply $3 $2
	  $14 = Local FMat4x4
	  Store $14 $13
	  $15 = Multiply $14 $12
	  $16 = Local Vec4
	  Store $16 $15
	  $17 = SV: ClipPosition
	  Store $17 $16
	  $18 = New Vec4($12)
	  $19 = Local Vec4
	  Store $19 $18
	  $20 = New Vec3($19)
	  $21 = Local Vec3
	  Store $21 $20
	  $22 = StageOutput 0: Smooth Vec3
	  Store $22 $21
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
	  $2 = Local UInt32
	  Store $2 $1
	  $3 = Local Float
	  Store $3 $0
	  $4 = Local Float
	  Store $4 $3
	  $5 = New Vec3($4, $4, $4)
	  $6 = Local Vec3
	  Store $6 $5
	  $7 = Local UInt32
	  $8 = 13
	  Store $7 $8
	  $9 = New UVec2($2, $7)
	  $10 = Local UVec2
	  Store $10 $9
	  $11 = New UVec2($10)
	  $12 = Local UVec2
	  Store $12 $11
	  $13 = New Vec3($6)
	  $14 = Local Vec3
	  Store $14 $13
	  $16 = Return 0: $15
	  Store $16 $14
	  $18 = Return 1: $17
	  Store $18 $12
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
	  Store $1 $0
	  $2 = Local Float
	  $3 = 0
	  Store $2 $3
	  $4 = New Vec3($2, $2, $2)
	  $5 = Local Vec3
	  Store $5 $4
	  $6 = Local Float
	  $7 = 1
	  Store $6 $7
	  $8 = Local Float
	  $9 = 1
	  Store $8 $9
	  $10 = New Vec3($8, $1, $6)
	  $11 = Local Vec3
	  Store $11 $10
	  $12 = Normalize($11)
	  $13 = Local Vec3
	  Store $13 $12
	  $15 = Return 0: $14
	  $16 = New Vec3($13)
	  $17 = Local Vec3
	  Store $17 $16
	  $18 = New Vec3($17)
	  $19 = Local Vec3
	  Store $19 $18
	  $20 = New Vec3($5)
	  $21 = Local Vec3
	  Store $21 $20
	  $22 = New Vec3($21)
	  $23 = Local Vec3
	  Store $23 $22
	  $24 = New Vec3($19)
	  $25 = Local Vec3
	  Store $25 $24
	  $26 = New Vec3($23)
	  $27 = Local Vec3
	  Store $27 $26
	  $28 = New Ray($27, $25)
	  Store $15 $28
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
	  $2 = Local Float
	  Store $2 $0
	  $3 = Local Vec3
	  sr1($2, $3)
	  $4 = Local UInt32
	  $5 = 2
	  Store $4 $5
	  $6 = Local Float
	  $7 = 1
	  Store $6 $7
	  $8 = Local Float
	  Store $8 $6
	  $9 = Local UInt32
	  Store $9 $4
	  $10 = Local Vec3
	  $11 = Local UVec2
	  sr2($8, $9, $10, $11)
	  $12 = New UVec2($10)
	  $13 = Local UVec2
	  Store $13 $12
	  $14 = New Vec3($11)
	  $15 = Local Vec3
	  Store $15 $14
	  $16 = Local Float
	  $17 = 2
	  Store $16 $17
	  $18 = Local Float
	  Store $18 $16
	  $19 = Local Ray
	  sr3($18, $19)
	  $20 = $19.origin
	  $21 = $19.direction
	  $22 = New Vec3($21)
	  $23 = Local Vec3
	  Store $23 $22
	  $24 = New Vec3($20)
	  $25 = Local Vec3
	  Store $25 $24
	  $26 = New UVec2($13)
	  $27 = Local UVec2
	  Store $27 $26
	  $28 = New Vec3($15)
	  $29 = Local Vec3
	  Store $29 $28
	  $30 = New Vec3($3)
	  $31 = Local Vec3
	  Store $31 $30
	  $32 = StageOutput 0: Smooth Vec3
	  Store $32 $31
	  $33 = StageOutput 1: Smooth Vec3
	  Store $33 $29
	  $34 = StageOutput 2: Smooth UVec2
	  Store $34 $27
	  $35 = StageOutput 3: Smooth Vec3
	  Store $35 $25
	  $36 = StageOutput 4: Smooth Vec3
	  Store $36 $23
	}
	)");
};
