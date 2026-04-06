#include <rcgp.hpp>

using namespace rcgp;

// Traced parameters are only valid for fragment, subroutine, or closest-hit shaders.

// Valid: traced params in fragment and subroutine
void traced_param_in_fragment()
{
	auto good = $shader(fragment)(vec3 position)
	{
		return vec4(position, 1.0f);
	};
}

void traced_param_in_subroutine()
{
	auto good = $subroutine(_tmp)(vec3 position)
	{
		return vec4(position, 1.0f);
	};
}

// Invalid: traced params in other stages

void traced_param_in_vertex()
{
	auto bad = $shader(vertex)(vec3 position, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void traced_param_in_compute()
{
	auto bad = $shader(compute)(vec3 position, WorkGroup <1> group) {};
}

void traced_param_in_mesh()
{
	auto bad = $shader(mesh)(vec3 position, WorkGroup <1> group)
	{
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126> payload;
		return payload;
	};
}
