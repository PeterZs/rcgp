#include <rcgp.hpp>

using namespace rcgp;

// MeshletPayload return is only valid for mesh shaders.
// MeshletPayload requires MaxVertices > 0 and MaxPrimitives > 0.

// Valid: MeshletPayload return in mesh
void meshlet_return_in_mesh()
{
	auto good = $shader(mesh)(WorkGroup <1> group)
	{
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126> payload;
		return payload;
	};
}

// Invalid: MeshletPayload in wrong stages or with bad parameters

void meshlet_return_in_vertex()
{
	auto bad = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126> payload;
		return payload;
	};
}

void meshlet_return_in_compute()
{
	auto bad = $shader(compute)(WorkGroup <1> group)
	{
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126> payload;
		return payload;
	};
}

void meshlet_zero_vertices()
{
	MeshletPayload <MeshPrimitive::eTriangles, 0, 126> bad;
}

void meshlet_zero_primitives()
{
	MeshletPayload <MeshPrimitive::eTriangles, 64, 0> bad;
}
