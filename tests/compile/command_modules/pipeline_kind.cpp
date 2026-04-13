#include <rcgp.hpp>

using namespace rcgp;

static UniformBuffer <float4> material;
static PushConstant <float4x4> mvp;

// Valid: dispatch is a fragment, fine without bind_pipeline in isolation
void kind_standalone_dispatch()
{
	auto cmds = nullptr | dispatch(1);
}

// Valid: draw is a fragment, fine without bind_pipeline in isolation
void kind_standalone_draw()
{
	auto cmds = nullptr | draw(3);
}

// Valid: rasterization pipeline + draw
void kind_rasterization(
	const Device &device, const ShaderCompiler &compiler, const RenderState &rs,
	const BoundDescriptor <material> &bound_mat,
	const ResourceTypeFor <mvp> &pc
)
{
	auto vs = $shader(vertex)(
		$contracts((v, mvp)),
		ClipPosition clip
	) {
		clip = v * float4(0, 0, 0, 1);
		return Smooth <float3> { float3(1) };
	};
	auto fs = $shader(fragment)(
		$contracts((m, material)),
		float3 nrm
	) {
		return float4(nrm, 1);
	};

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);

	auto cmds = nullptr
		| bind_pipeline(pipeline)
		| bind_descriptors(bound_mat)
		| bind_push_constants <mvp> (pc)
		| draw(3);
}

// Valid: compute pipeline + dispatch
void kind_compute(
	const Device &device, const ShaderCompiler &compiler
)
{
	auto cs = $shader(compute)(WorkGroup <1> group) {};

	auto pipeline = ComputeCombinator {
		.device = device,
		.compiler = compiler,
	} (cs);

	auto cmds = nullptr
		| bind_pipeline(pipeline)
		| dispatch(1);
}

// Invalid: rasterization pipeline bound but dispatch issued
void kind_wrong_rasterization_then_dispatch(
	const Device &device, const ShaderCompiler &compiler, const RenderState &rs
)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0); };
	auto fs = $shader(fragment)() { return float4(1); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);

	auto cmds = nullptr
		| bind_pipeline(pipeline)
		| dispatch(1);
}

// Invalid: compute pipeline bound but draw issued
void kind_wrong_compute_then_draw(
	const Device &device, const ShaderCompiler &compiler
)
{
	auto cs = $shader(compute)(WorkGroup <1> group) {};

	auto pipeline = ComputeCombinator {
		.device = device,
		.compiler = compiler,
	} (cs);

	auto cmds = nullptr
		| bind_pipeline(pipeline)
		| draw(3);
}

// Valid: live command buffer + bind_pipeline + draw
void kind_live_rasterization(
	const Device &device, const ShaderCompiler &compiler, const RenderState &rs
)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0); };
	auto fs = $shader(fragment)() { return float4(1); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);

	Commands <true> live {};
	auto cmds = live
		| bind_pipeline(pipeline)
		| draw(3);
}

// Invalid: draw at live-submission time with no pipeline ever bound.
// The Begin backstop injected by the Live|Deferred concat flips the
// sentinel into strict mode, catching the missing bind_pipeline.
void kind_live_draw_no_pipeline()
{
	Commands <true> live {};
	auto cmds = live | draw(3);
}

// Invalid: rasterization pipeline bound and a push constant bound in one
// chain, then dispatch issued in a subsequent concat. The intermediate
// NormalEntry from the push_constant makes this instantiation distinct from
// `kind_wrong_rasterization_then_dispatch`, and validates that
// current_pipeline propagates across the decode boundary.
void kind_deferred_multi_concat_mismatch(
	const Device &device, const ShaderCompiler &compiler, const RenderState &rs,
	const ResourceTypeFor <mvp> &pc
)
{
	auto vs = $shader(vertex)(
		$contracts((v, mvp)),
		ClipPosition clip
	) {
		clip = v * float4(0, 0, 0, 1);
	};
	auto fs = $shader(fragment)() { return float4(1); };

	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device,
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);

	auto stage1 = nullptr
		| bind_pipeline(pipeline)
		| bind_push_constants <mvp> (pc);
	auto cmds = stage1 | dispatch(1);
}
