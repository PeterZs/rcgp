#include <rcgp.hpp>

using namespace rcgp;

// Stage-locked intrinsics must only be used in their designated stage.

// Valid: intrinsics in their correct stages
void clipposition_in_vertex()
{
	auto good = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void vertex_index_in_vertex()
{
	auto good = $shader(vertex)(VertexIndex vi, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void global_invocation_in_compute()
{
	auto good = $shader(compute)(GlobalInvocationID gid, WorkGroup <1> group) {};
}

void local_invocation_in_compute()
{
	auto good = $shader(compute)(LocalInvocationID lid, WorkGroup <1> group) {};
}

void launch_id_in_raygen()
{
	auto good = $shader(raygen)(LaunchID lid, LaunchSize ls) {};
}

void hit_time_in_chit()
{
	auto good = $shader(chit)(HitTime ht, PrimitiveID pid) {};
}

// Invalid: intrinsics in wrong stages

void clipposition_in_fragment()
{
	auto bad = $shader(fragment)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return vec4(1.0f);
	};
}

void clipposition_in_compute()
{
	auto bad = $shader(compute)(ClipPosition clip, WorkGroup <1> group)
	{
		clip = vec4(0.0f);
	};
}

void vertex_index_in_fragment()
{
	auto bad = $shader(fragment)(VertexIndex vi)
	{
		return vec4(1.0f);
	};
}

void global_invocation_in_vertex()
{
	auto bad = $shader(vertex)(GlobalInvocationID gid, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void local_invocation_in_fragment()
{
	auto bad = $shader(fragment)(LocalInvocationID lid)
	{
		return vec4(1.0f);
	};
}

void launch_id_in_vertex()
{
	auto bad = $shader(vertex)(LaunchID lid, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void launch_size_in_compute()
{
	auto bad = $shader(compute)(LaunchSize ls, WorkGroup <1> group) {};
}

void hit_time_in_vertex()
{
	auto bad = $shader(vertex)(HitTime ht, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void primitive_id_in_compute()
{
	auto bad = $shader(compute)(PrimitiveID pid, WorkGroup <1> group) {};
}
