#include <rcgp.hpp>

using namespace rcgp;

struct MyOutput {
	vec3 position;
	f32 value;
	$reflection(position, value);
};

// Smooth/Flat/NoPerspective return is only valid for vertex shaders.
// Bare builtin return is only valid for vertex, fragment, or subroutine shaders.
// user_defined return is only valid for subroutine shaders.

// Valid: interpolation qualifiers from vertex
void smooth_return_in_vertex()
{
	auto good = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return Smooth(vec3(1.0f));
	};
}

void flat_return_in_vertex()
{
	auto good = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return Flat(u32(1));
	};
}

void noperspective_return_in_vertex()
{
	auto good = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return NoPerspective(vec3(1.0f));
	};
}

// Valid: builtin return from vertex, fragment, subroutine
void builtin_return_in_vertex()
{
	auto good = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return vec4(1.0f);
	};
}

void builtin_return_in_fragment()
{
	auto good = $shader(fragment)(vec3 color)
	{
		return vec4(color, 1.0f);
	};
}

void builtin_return_in_subroutine()
{
	auto good = $subroutine(_tmp)(vec3 input)
	{
		return vec4(input, 1.0f);
	};
}

// Valid: user_defined return from subroutine
void userdefined_return_in_subroutine()
{
	auto good = $subroutine(_tmp)(vec3 input)
	{
		return MyOutput {};
	};
}

// Invalid: return types in wrong stages

void smooth_return_in_fragment()
{
	auto bad = $shader(fragment)(vec3 color)
	{
		return Smooth(vec3(1.0f));
	};
}

void flat_return_in_fragment()
{
	auto bad = $shader(fragment)(vec3 color)
	{
		return Flat(u32(1));
	};
}

void noperspective_return_in_compute()
{
	auto bad = $shader(compute)(WorkGroup <1> group)
	{
		return NoPerspective(vec3(1.0f));
	};
}

void builtin_return_in_compute()
{
	auto bad = $shader(compute)(WorkGroup <1> group)
	{
		return vec4(1.0f);
	};
}

void builtin_return_in_mesh()
{
	auto bad = $shader(mesh)(WorkGroup <1> group)
	{
		return vec4(1.0f);
	};
}

void userdefined_return_in_vertex()
{
	auto bad = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return MyOutput {};
	};
}

void userdefined_return_in_fragment()
{
	auto bad = $shader(fragment)(vec3 color)
	{
		return MyOutput {};
	};
}
