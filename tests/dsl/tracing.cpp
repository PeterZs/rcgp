#include <rcgp.hpp>

#include "../block_diff.cpp"
#include "../suite.cpp"

using namespace rcgp;

std::string clean(const std::string &input)
{
	auto lines = split_lines(input);

	lines = std::vector(lines.begin() + 1, lines.end() - 1);

	std::string result;
	for (size_t i = 0; i < lines.size(); i++) {
		size_t off = 0;
		if (lines[i][0] == '\t')
			off++;

		result += lines[i].substr(off);
		if (i + 1 <  lines.size())
			result += '\n';
	}

	return result;
}

void assert_assembly_match(const SharedBlockReference &block, const std::string &str)
{
	auto expected = clean(str);
	auto act = generate_assembly(block);
	if (act != expected) {
		print_block_diff(expected, act);
		// TODO: --gt flag for testing to display ground truth
		// auto style = fmt::fg(fmt::color::gray)
		// 	| fmt::emphasis::italic;
		// fmt::print(style, "copy:\n{}\n", act);
		mark_fail;
	}
}

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
	  $0 = f32
	  $1 = local $0
	  $2 = 1
	  store $1 $2
	  $3 = float4
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
			// TODO: CTAD to infer, like Smooth { float3(1.0) }
			Smooth <float3> { float3(1.0) },
			Flat <uint2> { uint2(1, 4) },
		};
	};

	assert_assembly_match(vs, R"(
	block {
	  context {
	    model: vertex shader,
	    name: main,
	    thread out 0: $0 (smooth),
	    thread out 1: $1 (flat),
	  }
	  $2 = f32
	  $3 = local $2
	  $4 = 1
	  store $3 $4
	  $0 = float3
	  $5 = new $0($3, $3, $3)
	  $6 = local $0
	  store $6 $5
	  $7 = thread out($0, 0, smooth)
	  store $7 $6
	  $8 = u32
	  $9 = local $8
	  $10 = 1
	  store $9 $10
	  $11 = local $8
	  $12 = 4
	  store $11 $12
	  $1 = uint2
	  $13 = new $1($9, $11)
	  $14 = local $1
	  store $14 $13
	  $15 = thread out($1, 1, flat)
	  store $15 $14
	}
	)");
};
