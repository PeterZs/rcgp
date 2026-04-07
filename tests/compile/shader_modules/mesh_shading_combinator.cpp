#include <rcgp.hpp>

using namespace rcgp;

struct MeshOutputs {
	PerVertex <float3> color;
	$reflection(color);
};

// Valid: task + mesh + fragment with matching payload and IO
void mesh_shading_valid(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto ts = $shader(task)(TaskGroup <1> group) {
		TaskPayload <float3> payload;
		return payload;
	};
	auto ms = $shader(mesh)(TaskPayload <float3> payload, WorkGroup <1> group) {
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126, MeshOutputs> mp;
		return mp;
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = MeshShadingCombinator {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (ts, ms, fs);
}

// Invalid: vertex instead of task
void mesh_shading_vertex_as_task(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0.0f); };
	auto ms = $shader(mesh)(TaskPayload <float3> payload, WorkGroup <1> group) {
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126, MeshOutputs> mp;
		return mp;
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = MeshShadingCombinator {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, ms, fs);
}

// Invalid: task payload type mismatch
void mesh_shading_payload_mismatch(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto ts = $shader(task)(TaskGroup <1> group) {
		TaskPayload <float3> payload;
		return payload;
	};
	auto ms = $shader(mesh)(TaskPayload <float2> payload, WorkGroup <1> group) {
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126, MeshOutputs> mp;
		return mp;
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = MeshShadingCombinator {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (ts, ms, fs);
}

// Invalid: mesh outputs do not match fragment inputs
void mesh_shading_io_mismatch(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto ts = $shader(task)(TaskGroup <1> group) {
		TaskPayload <float3> payload;
		return payload;
	};
	auto ms = $shader(mesh)(TaskPayload <float3> payload, WorkGroup <1> group) {
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126, MeshOutputs> mp;
		return mp;
	};
	auto fs = $shader(fragment)(float2 uv) { return float4(uv, 0.0f, 1.0f); };

	auto pipeline = MeshShadingCombinator {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (ts, ms, fs);
}
