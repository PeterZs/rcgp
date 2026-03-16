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
	  $7 = 1
	  Store $6 $7
	  $8 = Add $0 $6
	  $9 = Local Int32
	  Store $9 $8
	  Store $0 $9
	  $10 = Local Int32
	  $11 = 1
	  Store $10 $11
	  $12 = Subtract $0 $10
	  $13 = Local Int32
	  Store $13 $12
	  Store $0 $13
	  $14 = Local Int32
	  $15 = 1
	  Store $14 $15
	  $16 = Subtract $0 $14
	  $17 = Local Int32
	  Store $17 $16
	  Store $0 $17
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
	  $3 = Add $1 $2
	  $4 = Local Float
	  Store $4 $3
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
	    $7 = Less $4 $0
	    $8 = Local Bool
	    Store $8 $7
	    $9 = LogicalNot $8
	    $10 = Local Bool
	    Store $10 $9
	    $11 = Block {
	      Break
	    }
	    Branch $10: $11
	    $12 = Local Float
	    Store $12 $4
	    $13 = Add $2 $12
	    $14 = Local Float
	    Store $14 $13
	    Store $2 $14
	    $15 = Add $4 $1
	    $16 = Local Int32
	    Store $16 $15
	    Store $4 $16
	  }
	  Loop $6
	  $18 = Return 0: $17
	  Store $18 $2
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
	    $7 = Less $4 $0
	    $8 = Local Bool
	    Store $8 $7
	    $9 = LogicalNot $8
	    $10 = Local Bool
	    Store $10 $9
	    $11 = Block {
	      Break
	    }
	    Branch $10: $11
	    $12 = Local Float
	    Store $12 $4
	    $13 = Add $2 $12
	    $14 = Local Float
	    Store $14 $13
	    Store $2 $14
	    $15 = Add $4 $1
	    $16 = Local Int32
	    Store $16 $15
	    Store $4 $16
	  }
	  Loop $6
	  $18 = Return 0: $17
	  Store $18 $2
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
	  $4 = Less $0 $2
	  $5 = Local Bool
	  Store $5 $4
	  $6 = Local Int32
	  $7 = 5
	  Store $6 $7
	  $8 = Greater $0 $6
	  $9 = Local Bool
	  Store $9 $8
	  $10 = LogicalAnd $5 $9
	  $11 = Local Bool
	  Store $11 $10
	  $12 = Local Int32
	  $13 = 11
	  Store $12 $13
	  $14 = Greater $0 $12
	  $15 = Local Bool
	  Store $15 $14
	  $16 = Block {
	    $17 = Local Int32
	    $18 = 1
	    Store $17 $18
	    $19 = Add $0 $17
	    $20 = Local Int32
	    Store $20 $19
	    Store $0 $20
	  }
	  $21 = Block {
	    $22 = Local Int32
	    $23 = 2
	    Store $22 $23
	    $24 = Add $0 $22
	    $25 = Local Int32
	    Store $25 $24
	    Store $0 $25
	  }
	  $26 = Block {
	    $27 = Local Int32
	    $28 = 3
	    Store $27 $28
	    $29 = Add $0 $27
	    $30 = Local Int32
	    Store $30 $29
	    Store $0 $30
	  }
	  Branch $15: $16, $11: $21, else: $26
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
