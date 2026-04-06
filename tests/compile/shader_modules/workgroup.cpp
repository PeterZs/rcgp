#include <rcgp.hpp>

using namespace rcgp;

struct PayloadData {
	u32 index;
	$reflection(index);
};

// WorkGroup is only valid for compute/mesh shaders.
// TaskGroup is only valid for task shaders.

// Valid: WorkGroup in compute and mesh, TaskGroup in task
void workgroup_in_compute()
{
	auto good = $shader(compute)(WorkGroup <8, 8, 1> group) {};
}

void workgroup_in_mesh()
{
	auto good = $shader(mesh)(WorkGroup <1> group)
	{
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126> payload;
		return payload;
	};
}

void taskgroup_in_task()
{
	auto good = $shader(task)(TaskGroup <1> group)
	{
		return TaskPayload <PayloadData> ();
	};
}

// Invalid: WorkGroup/TaskGroup in wrong stages

void workgroup_in_vertex()
{
	auto bad = $shader(vertex)(WorkGroup <8> group, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void workgroup_in_fragment()
{
	auto bad = $shader(fragment)(WorkGroup <8> group)
	{
		return vec4(1.0f);
	};
}

void workgroup_in_task()
{
	auto bad = $shader(task)(WorkGroup <1> group)
	{
		return TaskPayload <PayloadData> ();
	};
}

void taskgroup_in_compute()
{
	auto bad = $shader(compute)(TaskGroup <8> group) {};
}

void taskgroup_in_vertex()
{
	auto bad = $shader(vertex)(TaskGroup <1> group, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}
