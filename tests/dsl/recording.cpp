#include "common.hpp"

#define SUITE "recording"

// Recording:
// Testing that our DSL records the correct instructions into the Blocks.

struct RecordingPair {
	f32 x;
	i32 y;

	$reflection(x, y);
};

add_test(scalar_constant)
{
	auto sbr = record {
		i32 a = 3;
	};

	// TODO: match source location as well eventually
	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 3
	  Store $0 $1
	}
	)");
};

add_test(binary_op)
{
	auto sbr = record {
		i32 a = 1;
		i32 b = 2;
		i32 c = a + b;
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 1
	  Store $0 $1
	  $2 = Local Int32
	  $3 = 2
	  Store $2 $3
	  $4 = Add $0 $2
	  $5 = Local Int32
	  Store $5 $4
	}
	)");
};

add_test(unary_op)
{
	auto sbr = record {
		i32 a = 1;
		i32 b = -a;
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 1
	  Store $0 $1
	  $2 = Local Int32
	  $3 = -1
	  Store $2 $3
	  $4 = Multiply $2 $0
	  $5 = Local Int32
	  Store $5 $4
	}
	)");
};

add_test(assignment_store)
{
	auto sbr = record {
		i32 a;
		i32 b = 7;
		a = b;
	};
	
	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 7
	  Store $0 $1
	  $2 = Local Int32
	  Store $2 $0
	}
	)");
};

add_test(increment_decrement)
{
	auto sbr = record {
		i32 a = 1;
		++a;
		a++;
		--a;
		a--;
	};
	
	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 1
	  Store $0 $1
	  $2 = Local Int32
	  $3 = 1
	  Store $2 $3
	  $4 = Add $0 $2
	  $5 = Local Int32
	  Store $5 $4
	  Store $0 $5
	  $6 = Local Int32
	  Store $6 $0
	  $7 = Local Int32
	  $8 = 1
	  Store $7 $8
	  $9 = Add $0 $7
	  $10 = Local Int32
	  Store $10 $9
	  Store $0 $10
	  $11 = Local Int32
	  $12 = 1
	  Store $11 $12
	  $13 = Subtract $0 $11
	  $14 = Local Int32
	  Store $14 $13
	  Store $0 $14
	  $15 = Local Int32
	  Store $15 $0
	  $16 = Local Int32
	  $17 = 1
	  Store $16 $17
	  $18 = Subtract $0 $16
	  $19 = Local Int32
	  Store $19 $18
	  Store $0 $19
	}
	)");
};

add_test(type_caching)
{
	auto sbr = record {
		i32 a;
		i32 b;
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	}
	)");
};

add_test(construct_vec3)
{
	auto sbr = record {
		f32 x = 1.0f;
		f32 y = 2.0f;
		f32 z = 3.0f;
		vec3 v(x, y, z);
	};
	
	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Float
	  $1 = 1
	  Store $0 $1
	  $2 = Local Float
	  $3 = 2
	  Store $2 $3
	  $4 = Local Float
	  $5 = 3
	  Store $4 $5
	  $6 = New Vec3($0, $2, $4)
	  $7 = Local Vec3
	  Store $7 $6
	}
	)");
};

add_test(swizzle_xyz)
{
	auto sbr = record {
		vec4 v;
		vec3 xyz = v.xyz;
	};
	
	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $1 = Swizzle($0: xyz)
	  $2 = Local Vec3
	  Store $2 $1
	}
	)");
};

add_test(field_access)
{
	auto sbr = record {
		RecordingPair p;
		auto type = reconstruct_type <RecordingPair> ();
		p.override_reference(jems::local(type));
		auto x = p.x;
		auto y = p.y;
		f32 z = x + y;
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local RecordingPair
	  $1 = $0.x
	  $2 = $0.y
	  $3 = Local Float
	  Store $3 $1
	  $4 = Local Int32
	  Store $4 $2
	  $5 = Local Int32
	  Store $5 $4
	  $6 = Local Float
	  Store $6 $3
	  $7 = Add $6 $5
	  $8 = Local Float
	  Store $8 $7
	}
	)");
};

add_test(array_access)
{
	auto sbr = record {
		array <i32, 4> values;
		auto type = reconstruct_type <array <i32, 4>> ();
		values.override_reference(jems::local(type));
		i32 v = values[2];
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32[4]
	  $1 = Local Int32
	  $2 = 2
	  Store $1 $2
	  $3 = $0[$1]
	}
	)");
};

add_test(while_loop)
{
	auto sbr = record {
		auto typei32 = jems::type(Primitive::eInt32);
		auto typef32 = jems::type(Primitive::eFloat);

		i32 arg0;
		i32 arg1;
		arg0.override_reference(jems::argument(typei32, 0));
		arg1.override_reference(jems::argument(typei32, 1));

		f32 sum = 0;
		i32 i = 0;
		$while (i < arg0) {
			sum += i;
			i += arg1;
		};

		auto ret = jems::returns(typef32, 0);
		jems::store(ret, sum);
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Argument 0: Int32
	  $1 = Argument 1: Int32
	  $2 = Local Float
	  $3 = 0
	  Store $2 $3
	  $4 = Local Int32
	  $5 = 0
	  Store $4 $5
	  $6 = Block {
	    $7 = Local Int32
	    Store $7 $0
	    $8 = Local Int32
	    Store $8 $4
	    $9 = Less $8 $7
	    $10 = Local Bool
	    Store $10 $9
	    $11 = LogicalNot $10
	    $12 = Local Bool
	    Store $12 $11
	    $13 = Local Bool
	    Store $13 $12
	    $14 = Local Bool
	    Store $14 $13
	    $15 = Block {
	      Break
	    }
	    Branch $14: $15
	    $16 = Local Float
	    Store $16 $4
	    $17 = Add $2 $16
	    $18 = Local Float
	    Store $18 $17
	    Store $2 $18
	    $19 = Add $4 $1
	    $20 = Local Int32
	    Store $20 $19
	    Store $4 $20
	  }
	  Loop $6
	  $22 = Return 0: $21
	  Store $22 $2
	}
	)");
};

add_test(for_loop)
{
	auto sbr = record {
		auto typei32 = jems::type(Primitive::eInt32);
		auto typef32 = jems::type(Primitive::eFloat);

		i32 arg0;
		i32 arg1;
		arg0.override_reference(jems::argument(typei32, 0));
		arg1.override_reference(jems::argument(typei32, 1));

		f32 sum = 0;
		$for (i32 i = 0, i < arg0, i += arg1) {
			sum += i;
		};

		auto ret = jems::returns(typef32, 0);
		jems::store(ret, sum);
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Argument 0: Int32
	  $1 = Argument 1: Int32
	  $2 = Local Float
	  $3 = 0
	  Store $2 $3
	  $4 = Local Int32
	  $5 = 0
	  Store $4 $5
	  $6 = Block {
	    $7 = Local Int32
	    Store $7 $0
	    $8 = Local Int32
	    Store $8 $4
	    $9 = Less $8 $7
	    $10 = Local Bool
	    Store $10 $9
	    $11 = LogicalNot $10
	    $12 = Local Bool
	    Store $12 $11
	    $13 = Local Bool
	    Store $13 $12
	    $14 = Local Bool
	    Store $14 $13
	    $15 = Block {
	      Break
	    }
	    Branch $14: $15
	    $16 = Local Float
	    Store $16 $4
	    $17 = Add $2 $16
	    $18 = Local Float
	    Store $18 $17
	    Store $2 $18
	    $19 = Add $4 $1
	    $20 = Local Int32
	    Store $20 $19
	    Store $4 $20
	  }
	  Loop $6
	  $22 = Return 0: $21
	  Store $22 $2
	}
	)");
};

add_test(branching)
{
	auto sbr = record {
		i32 c = 12;
		$if (c > 11) {
			c += 1;
		} $elif (c < 11 and c > 5) {
			c += 2;
		} $else {
			c += 3;
		};
	};
	
	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 12
	  Store $0 $1
	  $2 = Local Int32
	  $3 = 11
	  Store $2 $3
	  $4 = Local Int32
	  Store $4 $0
	  $5 = Less $4 $2
	  $6 = Local Bool
	  Store $6 $5
	  $7 = Local Int32
	  $8 = 5
	  Store $7 $8
	  $9 = Local Int32
	  Store $9 $0
	  $10 = Greater $9 $7
	  $11 = Local Bool
	  Store $11 $10
	  $12 = LogicalAnd $6 $11
	  $13 = Local Bool
	  Store $13 $12
	  $14 = Local Bool
	  Store $14 $13
	  $15 = Local Int32
	  $16 = 11
	  Store $15 $16
	  $17 = Local Int32
	  Store $17 $0
	  $18 = Greater $17 $15
	  $19 = Local Bool
	  Store $19 $18
	  $20 = Local Bool
	  Store $20 $19
	  $21 = Local Bool
	  Store $21 $20
	  $22 = Local Bool
	  Store $22 $21
	  $23 = Local Bool
	  Store $23 $14
	  $24 = Local Bool
	  Store $24 $22
	  $25 = Block {
	    $26 = Local Int32
	    $27 = 1
	    Store $26 $27
	    $28 = Add $0 $26
	    $29 = Local Int32
	    Store $29 $28
	    Store $0 $29
	  }
	  $30 = Block {
	    $31 = Local Int32
	    $32 = 2
	    Store $31 $32
	    $33 = Add $0 $31
	    $34 = Local Int32
	    Store $34 $33
	    Store $0 $34
	  }
	  $35 = Block {
	    $36 = Local Int32
	    $37 = 3
	    Store $36 $37
	    $38 = Add $0 $36
	    $39 = Local Int32
	    Store $39 $38
	    Store $0 $39
	  }
	  Branch $24: $25, $23: $30, else: $35
	}
	)");
};

add_test(switch_branch)
{
	auto sbr = record {
		i32 m = 1;
		$switch (m)
		$case (i32(0)) {
			m += 10;
		}
		$case (i32(1)) {
			m += 20;
		}
		$default {
			m += 30;
		};
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Int32
	  $1 = 1
	  Store $0 $1
	  $2 = Local Int32
	  $3 = 1
	  Store $2 $3
	  $4 = Local Int32
	  Store $4 $2
	  $5 = Local Int32
	  $6 = 0
	  Store $5 $6
	  $7 = Local Int32
	  Store $7 $5
	  $8 = Local Int32
	  Store $8 $0
	  $9 = Local Int32
	  Store $9 $7
	  $10 = Local Int32
	  Store $10 $8
	  $11 = Equal $10 $9
	  $12 = Local Bool
	  Store $12 $11
	  $13 = Local Bool
	  Store $13 $12
	  $14 = Local Int32
	  Store $14 $8
	  $15 = Local Int32
	  Store $15 $4
	  $16 = Local Int32
	  Store $16 $14
	  $17 = Equal $16 $15
	  $18 = Local Bool
	  Store $18 $17
	  $19 = Local Bool
	  Store $19 $18
	  $20 = Local Int32
	  Store $20 $14
	  $21 = Local Int32
	  Store $21 $20
	  $22 = Block {
	    $23 = Local Int32
	    $24 = 10
	    Store $23 $24
	    $25 = Add $0 $23
	    $26 = Local Int32
	    Store $26 $25
	    Store $0 $26
	  }
	  $27 = Block {
	    $28 = Local Int32
	    $29 = 20
	    Store $28 $29
	    $30 = Add $0 $28
	    $31 = Local Int32
	    Store $31 $30
	    Store $0 $31
	  }
	  $32 = Block {
	    $33 = Local Int32
	    $34 = 30
	    Store $33 $34
	    $35 = Add $0 $33
	    $36 = Local Int32
	    Store $36 $35
	    Store $0 $36
	  }
	  Branch $13: $22, $19: $27, else: $32
	}
	)");
};

add_test(vector_store)
{
	auto sbr = record {
		float2 pos = float2(1) - 1;
		pos = float2(pos.x, -pos.y);
	};

	assert_assembly_match(sbr, R"(
	Block {
	  Context {
	    stage: Subroutine,
	    name: recorded,
	  }
	  $0 = Local Float
	  $1 = 1
	  Store $0 $1
	  $2 = New Vec2($0, $0)
	  $3 = Local Vec2
	  Store $3 $2
	  $4 = Local Float
	  $5 = 1
	  Store $4 $5
	  $6 = Subtract $3 $4
	  $7 = Local Vec2
	  Store $7 $6
	  $8 = Swizzle($7: y)
	  $9 = Local Float
	  Store $9 $8
	  $10 = Local Float
	  $11 = -1
	  Store $10 $11
	  $12 = Multiply $10 $9
	  $13 = Local Float
	  Store $13 $12
	  $14 = Swizzle($7: x)
	  $15 = Local Float
	  Store $15 $14
	  $16 = New Vec2($15, $13)
	  $17 = Local Vec2
	  Store $17 $16
	  Store $7 $17
	}
	)");
};
