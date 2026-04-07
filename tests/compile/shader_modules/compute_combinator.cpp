#include <rcgp.hpp>

using namespace rcgp;

// Valid: compute shader
void compute_valid(const Device &device, const ShaderCompiler &compiler)
{
	auto cs = $shader(compute)(WorkGroup <1> group) {};

	auto pipeline = ComputeCombinator {
		.device = device,
		.compiler = compiler,
	} (cs);
}

// Invalid: vertex shader as compute
void compute_vertex(const Device &device, const ShaderCompiler &compiler)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0.0f); };

	auto pipeline = ComputeCombinator {
		.device = device,
		.compiler = compiler,
	} (vs);
}

// Invalid: fragment shader as compute
void compute_fragment(const Device &device, const ShaderCompiler &compiler)
{
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto pipeline = ComputeCombinator {
		.device = device,
		.compiler = compiler,
	} (fs);
}
