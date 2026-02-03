#include "common.hpp"

#define SUITE "glsl"

// Resources
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

	assert_glsl_match(vs, R"(
	#version 460

	#extension GL_EXT_scalar_block_layout : require

	void main()
	{
	}
	)");
};

add_test(vs_clip)
{
	auto vs = $shader(vertex)(ClipPosition clip)
	{
		clip = float4(1);
	};

	assert_glsl_match(vs, R"(
	#version 460

	#extension GL_EXT_scalar_block_layout : require

	void main()
	{
	    float lvar0;
	    lvar0 = 1;
	    vec4 lvar1;
	    lvar1 = vec4(lvar0, lvar0, lvar0, lvar0);
	    gl_Position = lvar1;
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

	assert_glsl_match(vs, R"(
	#version 460

	#extension GL_EXT_scalar_block_layout : require

	layout (location = 0) smooth out vec3 lout0;
	layout (location = 1) flat out uvec2 lout1;

	void main()
	{
	    float lvar0;
	    lvar0 = 1;
	    vec3 lvar1;
	    lvar1 = vec3(lvar0, lvar0, lvar0);
	    lout0 = lvar1;
	    uint lvar2;
	    lvar2 = 4;
	    uint lvar3;
	    lvar3 = 1;
	    uvec2 lvar4;
	    lvar4 = uvec2(lvar3, lvar2);
	    lout1 = lvar4;
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

	assert_glsl_match(vs, R"(
	#version 460
	
	#extension GL_EXT_scalar_block_layout : require

	layout (location = 0) in vec3 lin0;

	layout (location = 0) smooth out vec3 lout0;

	void main()
	{
	    vec3 lvar0;
	    float lvar1;
	    lvar1 = 1;
	    vec4 lvar2;
	    lvar2 = vec4(lin0, lvar1);
	    gl_Position = lvar2;
	    lout0 = lin0;
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

	assert_glsl_match(vs, R"(
	#version 460

	#extension GL_EXT_scalar_block_layout : require

	struct View {
	    mat4 f0;
	    mat4 f1;
	    mat4 f2;
	};

	layout (location = 0) in vec3 lin0;

	layout (location = 0) smooth out vec3 lout0;

	layout (std430, push_constant) uniform PC {
	    layout (offset = 0) View pc;
	};

	void main()
	{
	    vec3 lvar0;
	    mat4 lvar1;
	    mat4 lvar2;
	    mat4 lvar3;
	    mat4 lvar4;
	    mat4 lvar5;
	    mat4 lvar6;
	    float lvar7;
	    lvar7 = 1;
	    vec4 lvar8;
	    lvar8 = vec4(lin0, lvar7);
	    gl_Position = ((pc.f2 * pc.f1) * (pc.f0 * lvar8));
	    vec3 lvar9;
	    lvar9 = vec3((pc.f0 * lvar8));
	    lout0 = lvar9;
	}
	)");
};

add_test(sr_return_primitives)
{
	auto sr = $subroutine(sr)() {
		return std::tuple { float3(1), uint2(12, 13) };
	};
	
	assert_glsl_match(sr, R"(
	void sr(out vec3 ret0, out uvec2 ret1)
	{
	    float lvar0;
	    lvar0 = 1;
	    vec3 lvar1;
	    lvar1 = vec3(lvar0, lvar0, lvar0);
	    uint lvar2;
	    lvar2 = 13;
	    uint lvar3;
	    lvar3 = 12;
	    uvec2 lvar4;
	    lvar4 = uvec2(lvar3, lvar2);
	    ret0 = lvar1;
	    ret1 = lvar4;
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
	
	assert_glsl_match(sr, R"(
	void sr(out Ray ret0)
	{
	    float lvar0;
	    lvar0 = 0;
	    vec3 lvar1;
	    lvar1 = vec3(lvar0, lvar0, lvar0);
	    float lvar2;
	    lvar2 = 1;
	    float lvar3;
	    lvar3 = 1;
	    float lvar4;
	    lvar4 = 1;
	    vec3 lvar5;
	    lvar5 = vec3(lvar4, lvar3, lvar2);
	    normalize(lvar5);
	    ret0 = Ray(lvar1, normalize(lvar5));
	}
	)");
};
