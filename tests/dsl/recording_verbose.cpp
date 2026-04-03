#include "common.hpp"

#define SUITE "recording_verbose"

// Recording (verbose):
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
	  $0 = Int32
	  $1 = Local $0
	  $2 = 3
	  Store $1 $2
	}
	)", true);
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
	  $0 = Int32
	  $1 = Local $0
	  $2 = 1
	  Store $1 $2
	  $3 = Local $0
	  $4 = 2
	  Store $3 $4
	  $5 = Add $1 $3
	  $6 = Local $0
	  Store $6 $5
	}
	)", true);
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
	  $0 = Int32
	  $1 = Local $0
	  $2 = 1
	  Store $1 $2
	  $3 = Local $0
	  $4 = -1
	  Store $3 $4
	  $5 = Multiply $3 $1
	  $6 = Local $0
	  Store $6 $5
	}
	)", true);
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
	  $0 = Int32
	  $1 = Local $0
	  $2 = 7
	  Store $1 $2
	  $3 = Local $0
	  Store $3 $1
	}
	)", true);
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
	  $0 = Int32
	  $1 = Local $0
	  $2 = 1
	  Store $1 $2
	  $3 = Local $0
	  $4 = 1
	  Store $3 $4
	  $5 = Add $1 $3
	  $6 = Local $0
	  Store $6 $5
	  Store $1 $6
	  $7 = Local $0
	  Store $7 $1
	  $8 = Local $0
	  $9 = 1
	  Store $8 $9
	  $10 = Add $1 $8
	  $11 = Local $0
	  Store $11 $10
	  Store $1 $11
	  $12 = Local $0
	  $13 = 1
	  Store $12 $13
	  $14 = Subtract $1 $12
	  $15 = Local $0
	  Store $15 $14
	  Store $1 $15
	  $16 = Local $0
	  Store $16 $1
	  $17 = Local $0
	  $18 = 1
	  Store $17 $18
	  $19 = Subtract $1 $17
	  $20 = Local $0
	  Store $20 $19
	  Store $1 $20
	}
	)", true);
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
	)", true);
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
	  $0 = Float
	  $1 = Local $0
	  $2 = 1
	  Store $1 $2
	  $3 = Local $0
	  $4 = 2
	  Store $3 $4
	  $5 = Local $0
	  $6 = 3
	  Store $5 $6
	  $7 = Vec3
	  $8 = New $7($1, $3, $5)
	  $9 = Local $7
	  Store $9 $8
	}
	)", true);
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
	  $2 = Vec3
	  $3 = Local $2
	  Store $3 $1
	}
	)", true);
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
	  $0 = Float
	  $1 = Int32
	  $2 = RecordingPair { x: $0, y: $1 }
	  $3 = Local $2
	  $4 = $3.x
	  $5 = $3.y
	  $6 = Local $0
	  Store $6 $4
	  $7 = Local $1
	  Store $7 $5
	  $8 = Local $1
	  Store $8 $7
	  $9 = Local $0
	  Store $9 $6
	  $10 = Add $9 $8
	  $11 = Local $0
	  Store $11 $10
	}
	)", true);
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
	  $0 = Int32
	  $1 = Int32[4]
	  $2 = Local $1
	  $3 = Local $0
	  $4 = 2
	  Store $3 $4
	  $5 = $2[$3]
	}
	)", true);
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
	  $0 = Int32
	  $1 = Float
	  $2 = Argument 0: $0
	  $3 = Argument 1: $0
	  $4 = Local $1
	  $5 = 0
	  Store $4 $5
	  $6 = Local $0
	  $7 = 0
	  Store $6 $7
	  $8 = Block {
	    $1 = Float
	    $0 = Int32
	    $9 = Local $0
	    Store $9 $2
	    $10 = Local $0
	    Store $10 $6
	    $11 = Less $10 $9
	    $12 = Bool
	    $13 = Local $12
	    Store $13 $11
	    $14 = LogicalNot $13
	    $15 = Local $12
	    Store $15 $14
	    $16 = Local $12
	    Store $16 $15
	    $17 = Local $12
	    Store $17 $16
	    $18 = Block {
	      Break
	    }
	    Branch $17: $18
	    $19 = Local $1
	    Store $19 $6
	    $20 = Add $4 $19
	    $21 = Local $1
	    Store $21 $20
	    Store $4 $21
	    $22 = Add $6 $3
	    $23 = Local $0
	    Store $23 $22
	    Store $6 $23
	  }
	  Loop $8
	  $24 = Return 0: $1
	  Store $24 $4
	}
	)", true);
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
	  $0 = Int32
	  $1 = Float
	  $2 = Argument 0: $0
	  $3 = Argument 1: $0
	  $4 = Local $1
	  $5 = 0
	  Store $4 $5
	  $6 = Local $0
	  $7 = 0
	  Store $6 $7
	  $8 = Block {
	    $1 = Float
	    $0 = Int32
	    $9 = Local $0
	    Store $9 $2
	    $10 = Local $0
	    Store $10 $6
	    $11 = Less $10 $9
	    $12 = Bool
	    $13 = Local $12
	    Store $13 $11
	    $14 = LogicalNot $13
	    $15 = Local $12
	    Store $15 $14
	    $16 = Local $12
	    Store $16 $15
	    $17 = Local $12
	    Store $17 $16
	    $18 = Block {
	      Break
	    }
	    Branch $17: $18
	    $19 = Local $1
	    Store $19 $6
	    $20 = Add $4 $19
	    $21 = Local $1
	    Store $21 $20
	    Store $4 $21
	    $22 = Add $6 $3
	    $23 = Local $0
	    Store $23 $22
	    Store $6 $23
	  }
	  Loop $8
	  $24 = Return 0: $1
	  Store $24 $4
	}
	)", true);
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
	  $0 = Int32
	  $1 = Local $0
	  $2 = 12
	  Store $1 $2
	  $3 = Local $0
	  $4 = 11
	  Store $3 $4
	  $5 = Local $0
	  Store $5 $1
	  $6 = Less $5 $3
	  $7 = Bool
	  $8 = Local $7
	  Store $8 $6
	  $9 = Local $0
	  $10 = 5
	  Store $9 $10
	  $11 = Local $0
	  Store $11 $1
	  $12 = Greater $11 $9
	  $13 = Local $7
	  Store $13 $12
	  $14 = LogicalAnd $8 $13
	  $15 = Local $7
	  Store $15 $14
	  $16 = Local $7
	  Store $16 $15
	  $17 = Local $0
	  $18 = 11
	  Store $17 $18
	  $19 = Local $0
	  Store $19 $1
	  $20 = Greater $19 $17
	  $21 = Local $7
	  Store $21 $20
	  $22 = Local $7
	  Store $22 $21
	  $23 = Local $7
	  Store $23 $22
	  $24 = Local $7
	  Store $24 $23
	  $25 = Local $7
	  Store $25 $16
	  $26 = Local $7
	  Store $26 $24
	  $27 = Block {
	    $0 = Int32
	    $28 = Local $0
	    $29 = 1
	    Store $28 $29
	    $30 = Add $1 $28
	    $31 = Local $0
	    Store $31 $30
	    Store $1 $31
	  }
	  $32 = Block {
	    $0 = Int32
	    $33 = Local $0
	    $34 = 2
	    Store $33 $34
	    $35 = Add $1 $33
	    $36 = Local $0
	    Store $36 $35
	    Store $1 $36
	  }
	  $37 = Block {
	    $0 = Int32
	    $38 = Local $0
	    $39 = 3
	    Store $38 $39
	    $40 = Add $1 $38
	    $41 = Local $0
	    Store $41 $40
	    Store $1 $41
	  }
	  Branch $26: $27, $25: $32, else: $37
	}
	)", true);
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
	  $0 = Float
	  $1 = Local $0
	  $2 = 1
	  Store $1 $2
	  $3 = Vec2
	  $4 = New $3($1, $1)
	  $5 = Local $3
	  Store $5 $4
	  $6 = Local $0
	  $7 = 1
	  Store $6 $7
	  $8 = Subtract $5 $6
	  $9 = Local $3
	  Store $9 $8
	  $10 = Swizzle($9: y)
	  $11 = Local $0
	  Store $11 $10
	  $12 = Local $0
	  $13 = -1
	  Store $12 $13
	  $14 = Multiply $12 $11
	  $15 = Local $0
	  Store $15 $14
	  $16 = Swizzle($9: x)
	  $17 = Local $0
	  Store $17 $16
	  $18 = New $3($17, $15)
	  $19 = Local $3
	  Store $19 $18
	  Store $9 $19
	}
	)", true);
};
