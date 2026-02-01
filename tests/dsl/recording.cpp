#include <cstdint>
#include <memory>
#include <unordered_map>

#include <rcgp.hpp>

#include "../suite.cpp"

using namespace rcgp;

struct RecordingPair {
	f32 x;
	i32 y;

	$reflection(x, y);
};

SharedBlockReference trace_block(const std::function <void ()> &fn)
{
	Tracer::singleton.type_cache.clear();
	auto sbr = std::make_shared <Block> ();
	{
		jems::scope scope(sbr);
		fn();
	}
	return sbr;
}

// TODO: mutch of this is probably useful in the main code
bool match_ref(
	const Reference &actual,
	const Reference &expected,
	const std::unordered_map <const Instruction *, const Instruction *> &map
)
{
	if (!expected)
		return actual.get() == nullptr;
	auto it = map.find(expected.get());
	if (it == map.end())
		return false;
	return it->second == actual.get();
}

bool match_primitive(const PrimitiveType &actual, const PrimitiveType &expected)
{
	return actual.index() == expected.index();
}

bool match_constant(const Constant &actual, const Constant &expected)
{
	if (actual.index() != expected.index())
		return false;
	return std::visit([](const auto &a, const auto &b) {
		if constexpr (std::is_same_v <std::decay_t <decltype(a)>, std::decay_t <decltype(b)>>)
			return a == b;
		else
			return false;
	}, actual, expected);
}

bool match_type(
	const Type &actual,
	const Type &expected,
	const std::unordered_map <const Instruction *, const Instruction *> &map
)
{
	if (actual.index() != expected.index())
		return false;
	vswitch (actual) {
	vcase(PrimitiveType): {
		return match_primitive(actual.as <PrimitiveType> (), expected.as <PrimitiveType> ());
	}
	vcase(ArrayType): {
		const auto &a = actual.as <ArrayType> ();
		const auto &b = expected.as <ArrayType> ();
		if (a.size != b.size)
			return false;
		return match_ref(a.base, b.base, map);
	}
	vcase(AggregateType): {
		const auto &a = actual.as <AggregateType> ();
		const auto &b = expected.as <AggregateType> ();
		if (a.name != b.name || a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); i++) {
			if (!match_ref(a[i], b[i], map))
				return false;
		}
		return true;
	}
	default:
		break;
	}
	return false;
}

bool match_instruction(
	const Instruction &actual,
	const Instruction &expected,
	const std::unordered_map <const Instruction *, const Instruction *> &map
)
{
	if (actual.index() != expected.index())
		return false;
	vswitch (actual) {
	vcase(Constant): {
		return match_constant(actual.as <Constant> (), expected.as <Constant> ());
	}
	vcase(Operation): {
		const auto &a = actual.as <Operation> ();
		const auto &b = expected.as <Operation> ();
		return a.code == b.code
			&& match_ref(a.a, b.a, map)
			&& match_ref(a.b, b.b, map);
	}
	vcase(Construct): {
		const auto &a = actual.as <Construct> ();
		const auto &b = expected.as <Construct> ();
		if (!match_ref(a.type, b.type, map) || a.args.size() != b.args.size())
			return false;
		for (size_t i = 0; i < a.args.size(); i++) {
			if (!match_ref(a.args[i], b.args[i], map))
				return false;
		}
		return true;
	}
	vcase(Local): {
		const auto &a = actual.as <Local> ();
		const auto &b = expected.as <Local> ();
		return match_ref(a.type, b.type, map);
	}
	vcase(Store): {
		const auto &a = actual.as <Store> ();
		const auto &b = expected.as <Store> ();
		return match_ref(a.destination, b.destination, map)
			&& match_ref(a.source, b.source, map);
	}
	vcase(Swizzle): {
		const auto &a = actual.as <Swizzle> ();
		const auto &b = expected.as <Swizzle> ();
		return a.code == b.code && match_ref(a.value, b.value, map);
	}
	vcase(FieldAccess): {
		const auto &a = actual.as <FieldAccess> ();
		const auto &b = expected.as <FieldAccess> ();
		return a.fidx == b.fidx && match_ref(a.value, b.value, map);
	}
	vcase(ArrayAccess): {
		const auto &a = actual.as <ArrayAccess> ();
		const auto &b = expected.as <ArrayAccess> ();
		return match_ref(a.value, b.value, map)
			&& match_ref(a.index, b.index, map);
	}
	vcase(Type): {
		return match_type(actual.as <Type> (), expected.as <Type> (), map);
	}
	default:
		break;
	}
	return false;
}

bool match_block(const Block &actual, const Block &expected)
{
	if (actual.size() != expected.size())
		return false;
	std::unordered_map <const Instruction *, const Instruction *> map;
	map.reserve(expected.size());
	for (size_t i = 0; i < actual.size(); i++) {
		const auto &a = actual[i];
		const auto &b = expected[i];
		if (!a || !b)
			return a.get() == b.get();
		if (!match_instruction(*a, *b, map))
			return false;
		map.emplace(b.get(), a.get());
	}
	return true;
}

void assert_blocks_match(
	const SharedBlockReference &actual,
	const SharedBlockReference &expected
)
{
	if (actual.get() == nullptr || expected.get() == nullptr) {
		mark_fail;
	} else if (not match_block(*actual, *expected)) {
		auto act = generate_assembly(actual);
		auto exp = generate_assembly(expected);
		fmt::println("mismatching blocks:\n{}\n{}", act, exp);
		mark_fail;
	}
}

add_test(scalar_constant)
{
	auto sbr = trace_block([] {
		i32 a = 3;
	});

	auto expected = trace_block([] {
		auto type = jems::type(int32_t());
		auto local = jems::local(type);
		auto value = jems::constant(int32_t(3));
		jems::store(local, value);
	});

	assert_blocks_match(sbr, expected);
};

add_test(binary_op)
{
	auto sbr = trace_block([&] {
		i32 a = 1;
		i32 b = 2;
		i32 c = a + b;
	});

	auto expected = trace_block([] {
		auto type = jems::type(int32_t());
		auto a = jems::local(type);
		auto one = jems::constant(int32_t(1));
		jems::store(a, one);
		auto b = jems::local(type);
		auto two = jems::constant(int32_t(2));
		jems::store(b, two);
		jems::operation(OperationCode::eAdd, a, b);
	});

	assert_blocks_match(sbr, expected);
};

add_test(unary_op)
{
	auto sbr = trace_block([&] {
		i32 a = 1;
		i32 b = -a;
	});
	auto expected = trace_block([] {
		auto type = jems::type(int32_t());
		auto a = jems::local(type);
		auto one = jems::constant(int32_t(1));
		jems::store(a, one);
		auto neg = jems::local(type);
		auto minus_one = jems::constant(int32_t(-1));
		jems::store(neg, minus_one);
		jems::operation(OperationCode::eMultiply, neg, a);
	});

	assert_blocks_match(sbr, expected);
};

add_test(assignment_store)
{
	auto sbr = trace_block([&] {
		i32 a;
		i32 b = 7;
		a = b;
	});
	
	auto expected = trace_block([] {
		auto type = jems::type(int32_t());
		auto a = jems::local(type);
		auto b = jems::local(type);
		auto seven = jems::constant(int32_t(7));
		jems::store(b, seven);
		jems::store(a, b);
	});

	assert_blocks_match(sbr, expected);
};

add_test(increment_decrement)
{
	auto sbr = trace_block([&] {
		i32 a = 1;
		++a;
		a++;
		--a;
		a--;
	});
	
	auto expected = trace_block([] {
		auto type = jems::type(int32_t());
		auto a = jems::local(type);
		auto one = jems::constant(int32_t(1));
		jems::store(a, one);

		auto inc1 = jems::local(type);
		auto incv1 = jems::constant(int32_t(1));
		jems::store(inc1, incv1);
		auto add1 = jems::operation(OperationCode::eAdd, a, inc1);
		jems::store(a, add1);

		auto inc2 = jems::local(type);
		auto incv2 = jems::constant(int32_t(1));
		jems::store(inc2, incv2);
		auto add2 = jems::operation(OperationCode::eAdd, a, inc2);
		jems::store(a, add2);

		auto dec1 = jems::local(type);
		auto decv1 = jems::constant(int32_t(1));
		jems::store(dec1, decv1);
		auto sub1 = jems::operation(OperationCode::eSubtract, a, dec1);
		jems::store(a, sub1);

		auto dec2 = jems::local(type);
		auto decv2 = jems::constant(int32_t(1));
		jems::store(dec2, decv2);
		auto sub2 = jems::operation(OperationCode::eSubtract, a, dec2);
		jems::store(a, sub2);
	});

	assert_blocks_match(sbr, expected);
};

add_test(type_caching)
{
	auto sbr = trace_block([] {
		i32 a;
		i32 b;
	});

	auto expected = trace_block([] {
		auto type = jems::type(int32_t());
		auto a = jems::local(type);
		auto b = jems::local(type);
	});
	
	assert_blocks_match(sbr, expected);
};

add_test(construct_vec3)
{
	auto sbr = trace_block([] {
		f32 x = 1.0f;
		f32 y = 2.0f;
		f32 z = 3.0f;
		vec3 v(x, y, z);
	});
	
	auto expected = trace_block([] {
		auto f32_type = jems::type(float());
		auto x = jems::local(f32_type);
		auto xval = jems::constant(1.0f);
		jems::store(x, xval);
		auto y = jems::local(f32_type);
		auto yval = jems::constant(2.0f);
		jems::store(y, yval);
		auto z = jems::local(f32_type);
		auto zval = jems::constant(3.0f);
		jems::store(z, zval);

		auto vec3_type = jems::type(VectorType <float, 3> ());
		auto ctor = jems::construct(vec3_type, x, y, z);
		auto v = jems::local(vec3_type);
		jems::store(v, ctor);
	});

	assert_blocks_match(sbr, expected);
};

add_test(swizzle_xyz)
{
	auto sbr = trace_block([] {
		vec4 v;
		vec3 xyz = v.xyz;
	});
	
	auto expected = trace_block([] {
		auto vec4_type = jems::type(VectorType <float, 4> ());
		auto v = jems::local(vec4_type);
		jems::swizzle(SwizzleCode::eXYZ, v);
	});

	assert_blocks_match(sbr, expected);
};

add_test(field_access)
{
	auto sbr = trace_block([] {
		RecordingPair p;
		auto type = reconstruct_type <RecordingPair> ();
		inject_reference(p, jems::local(type));
		auto x = p.x;
		auto y = p.y;
	});

	auto expected = trace_block([] {
		auto f32_type = jems::type(float());
		auto x = jems::local(f32_type);
		auto i32_type = jems::type(int32_t());
		auto y = jems::local(i32_type);
		(void)x;
		(void)y;

		AggregateType aggregate;
		aggregate.name = "RecordingPair";
		aggregate.emplace_back(f32_type);
		aggregate.emplace_back(i32_type);
		auto pair_type = jems::type(aggregate);
		auto pair = jems::local(pair_type);
		jems::field_access(pair, 0);
		jems::field_access(pair, 1);
	});

	assert_blocks_match(sbr, expected);
};

add_test(array_access)
{
	auto sbr = trace_block([] {
		array <i32, 4> values;
		auto type = reconstruct_type <array <i32, 4>> ();
		inject_reference(values, jems::local(type));
		i32 v = values[2];
	});

	auto expected = trace_block([] {
		auto i32_type = jems::type(int32_t());
		auto arr_type = jems::type(ArrayType(i32_type, 4));
		auto arr = jems::local(arr_type);
		auto v = jems::local(i32_type);
		(void)v;
		auto idx = jems::local(i32_type);
		auto idx_val = jems::constant(int32_t(2));
		jems::store(idx, idx_val);
		jems::array_access(arr, idx);
	});

	assert_blocks_match(sbr, expected);
};
