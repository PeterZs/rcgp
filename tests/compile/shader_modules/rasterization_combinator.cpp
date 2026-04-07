#include <rcgp.hpp>

using namespace rcgp;

// Valid: vertex + fragment with matching IO
void raster_vertex_fragment(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto vs = $shader(vertex)(ClipPosition clip) {
		clip = float4(0.0f);
		return Smooth(float3(1.0f));
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);
}

// Valid: vertex only (shadow/depth pass)
void raster_vertex_only(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0.0f); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs);
}

// Invalid: fragment + vertex (wrong order)
void raster_wrong_order(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto vs = $shader(vertex)(ClipPosition clip) {
		clip = float4(0.0f);
		return Smooth(float3(1.0f));
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (fs, vs);
}

// Invalid: compute shader instead of vertex
void raster_compute_as_vertex(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto cs = $shader(compute)(WorkGroup <1> group) {};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (cs, fs);
}

// Invalid: vertex outputs do not match fragment inputs
void raster_io_mismatch(const Device &device, const ShaderCompiler &compiler, const RenderState &rs)
{
	auto vs = $shader(vertex)(ClipPosition clip) {
		clip = float4(0.0f);
		return Smooth(float3(1.0f));
	};
	auto fs = $shader(fragment)(float2 uv) { return float4(uv, 0.0f, 1.0f); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);
}
