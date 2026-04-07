#include <rcgp.hpp>

using namespace rcgp;

static TraceGroup <float3> radiance;
static TraceGroup <f32> occlusion;

// Valid: raygen dispatches radiance, miss/chit receive radiance
void raytracing_valid(const Device &device, const ShaderCompiler &compiler)
{
	auto rgen = $shader(raygen)(
		$contracts((rad, dispatcher <radiance>)),
		LaunchID idx
	) {};
	auto miss = $shader(miss)($contracts((rad, receiver <radiance>))) {};
	auto chit = $shader(chit)($contracts((rad, receiver <radiance>)), float2 bary) {};

	auto pipeline = RayTracingCombinator {
		.device = device,
		.compiler = compiler,
	} (rgen, std::tuple { miss }, std::tuple { chit });
}

// Invalid: vertex shader as raygen
void raytracing_vertex_as_raygen(const Device &device, const ShaderCompiler &compiler)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0.0f); };
	auto miss = $shader(miss)($contracts((rad, receiver <radiance>))) {};
	auto chit = $shader(chit)($contracts((rad, receiver <radiance>)), float2 bary) {};

	auto pipeline = RayTracingCombinator {
		.device = device,
		.compiler = compiler,
	} (vs, std::tuple { miss }, std::tuple { chit });
}

// Invalid: receiver has no matching dispatcher
void raytracing_missing_dispatcher(const Device &device, const ShaderCompiler &compiler)
{
	auto rgen = $shader(raygen)(
		$contracts((rad, dispatcher <radiance>)),
		LaunchID idx
	) {};
	auto miss = $shader(miss)($contracts((rad, receiver <radiance>))) {};
	auto chit = $shader(chit)(
		$contracts((rad, receiver <radiance>), (occ, receiver <occlusion>)),
		float2 bary
	) {};

	auto pipeline = RayTracingCombinator {
		.device = device,
		.compiler = compiler,
	} (rgen, std::tuple { miss }, std::tuple { chit });
}
