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
	assert_assembly_eq(sbr, "recording_verbose/scalar_constant.asm", true);
};

add_test(binary_op)
{
	auto sbr = record {
		i32 a = 1;
		i32 b = 2;
		i32 c = a + b;
	};

	assert_assembly_eq(sbr, "recording_verbose/binary_op.asm", true);
};

add_test(unary_op)
{
	auto sbr = record {
		i32 a = 1;
		i32 b = -a;
	};

	assert_assembly_eq(sbr, "recording_verbose/unary_op.asm", true);
};

add_test(assignment_store)
{
	auto sbr = record {
		i32 a;
		i32 b = 7;
		a = b;
	};
	
	assert_assembly_eq(sbr, "recording_verbose/assignment_store.asm", true);
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
	
	assert_assembly_eq(sbr, "recording_verbose/increment_decrement.asm", true);
};

add_test(type_caching)
{
	auto sbr = record {
		i32 a;
		i32 b;
	};

	assert_assembly_eq(sbr, "recording_verbose/type_caching.asm", true);
};

add_test(construct_vec3)
{
	auto sbr = record {
		f32 x = 1.0f;
		f32 y = 2.0f;
		f32 z = 3.0f;
		vec3 v(x, y, z);
	};
	
	assert_assembly_eq(sbr, "recording_verbose/construct_vec3.asm", true);
};

add_test(swizzle_xyz)
{
	auto sbr = record {
		vec4 v;
		vec3 xyz = v.xyz;
	};
	
	assert_assembly_eq(sbr, "recording_verbose/swizzle_xyz.asm", true);
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

	assert_assembly_eq(sbr, "recording_verbose/field_access.asm", true);
};

add_test(array_access)
{
	auto sbr = record {
		array <i32, 4> values;
		auto type = reconstruct_type <array <i32, 4>> ();
		values.override_reference(jems::local(type));
		i32 v = values[2];
	};

	assert_assembly_eq(sbr, "recording_verbose/array_access.asm", true);
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

	assert_assembly_eq(sbr, "recording_verbose/while_loop.asm", true);
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

	assert_assembly_eq(sbr, "recording_verbose/for_loop.asm", true);
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
	
	assert_assembly_eq(sbr, "recording_verbose/branching.asm", true);
};

add_test(vector_store)
{
	auto sbr = record {
		float2 pos = float2(1) - 1;
		pos = float2(pos.x, -pos.y);
	};

	assert_assembly_eq(sbr, "recording_verbose/vector_store.asm", true);
};
