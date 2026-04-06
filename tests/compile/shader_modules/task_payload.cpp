#include <rcgp.hpp>

using namespace rcgp;

struct PayloadData {
	u32 index;
	$reflection(index);
};

// TaskPayload as a parameter is only valid for mesh shaders.
// TaskPayload as a return type is only valid for task shaders.

// Valid: TaskPayload param in mesh, return in task
void taskpayload_param_in_mesh()
{
	auto good = $shader(mesh)(TaskPayload <PayloadData> payload, WorkGroup <1> group)
	{
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126> out;
		return out;
	};
}

void taskpayload_return_in_task()
{
	auto good = $shader(task)(TaskGroup <1> group)
	{
		return TaskPayload <PayloadData> ();
	};
}

// Invalid: TaskPayload in other stages

void taskpayload_param_in_vertex()
{
	auto bad = $shader(vertex)(TaskPayload <PayloadData> payload, ClipPosition clip)
	{
		clip = vec4(0.0f);
	};
}

void taskpayload_param_in_compute()
{
	auto bad = $shader(compute)(TaskPayload <PayloadData> payload, WorkGroup <1> group) {};
}

void taskpayload_return_in_vertex()
{
	auto bad = $shader(vertex)(ClipPosition clip)
	{
		clip = vec4(0.0f);
		return TaskPayload <PayloadData> ();
	};
}

void taskpayload_return_in_compute()
{
	auto bad = $shader(compute)(WorkGroup <1> group)
	{
		return TaskPayload <PayloadData> ();
	};
}
