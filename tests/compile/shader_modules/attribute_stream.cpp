#include <rcgp.hpp>

using namespace rcgp;

static AttributeStream <float3> position;

// AttributeStream is only valid for vertex or subroutine shaders.

// Valid: AttributeStream in vertex
void attribute_stream_in_vertex()
{
	auto good = $shader(vertex)($contracts(position), ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

// Invalid: AttributeStream in other stages

void attribute_stream_in_fragment()
{
	auto bad = $shader(fragment)($contracts(position))
	{
		return vec4(1.0f);
	};
}

void attribute_stream_in_compute()
{
	auto bad = $shader(compute)($contracts(position), WorkGroup <1> group) {};
}
