#include "common.hpp"
#include "common_resources.hpp"

#include <dsl/optimization.hpp>

#define SUITE "optimization"

// Optimization:
// Testing that the optimization pass yields the correct instructions.

static auto flags = OptimizationPhases::eStable;
static auto readable_flags = flags | OptimizationPhases::eReadability;

add_test(vs_empty)
{
	auto vs = $shader(vertex)() {};

	optimize(vs, OptimizationPhases::eDeadCodeElimination);

	assert_glsl_eq(vs, "optimization/vs_empty.glsl");
};

add_test(vs_clip)
{
	auto vs = $shader(vertex)(ClipPosition clip)
	{
		clip = float4(1);
	};

	optimize(vs, flags);

	assert_glsl_eq(vs, "optimization/vs_clip.glsl");
};

add_test(vs_louts)
{
	auto vs = $shader(vertex)()
	{
		return std::tuple {
			Smooth { float3(1.0) },
			Flat { uint2(1, 4) },
		};
	};
	
	optimize(vs, flags);

	assert_glsl_eq(vs, "optimization/vs_louts.glsl");
};

add_test(vs_stream)
{
	auto vs = $shader(vertex)(
		$contracts((p, fwd::position)),
		ClipPosition cpos
	) -> float3
	{
		cpos = float4(p, 1);
		return p;
	};
	
	optimize(vs, flags);

	assert_glsl_eq(vs, "optimization/vs_stream.glsl");
};

add_test(vs_multiple_io)
{
	auto vs = $shader(vertex)(
		$contracts(
			(pos, fwd::position),
			(nrm, fwd::normal),
			(uv, fwd::uv)
		)
	) {
		return std::tuple {
			Smooth <float3> { pos },
			Smooth <float3> { nrm },
			Smooth <float2> { uv },
		};
	};
	
	optimize(vs, flags);
	
	assert_glsl_eq(vs, "optimization/vs_multiple_io.glsl");
};

add_test(vs_push_constant)
{
	auto vs = $shader(vertex)(
		$contracts(
			(view, fwd::view),
			(pos, fwd::position)
		),
		ClipPosition cpos
	) -> float3
	{
		float4 wpos = view.model * float4(pos, 1);
		cpos = view.proj * view.view * wpos;
		return float3(wpos);
	};
	
	optimize(vs, flags);

	assert_glsl_eq(vs, "optimization/vs_push_constant.glsl");
};

add_test(fr_diffuse_lighting)
{
	auto fs = $shader(fragment)(
		$contracts(
			(lights, fwd::lights),
			(material, fwd::material)
		),
		float3 position,
		float3 normal,
		float2 uv
	) -> float3
	{
		vec3 base = material.albedo.sample(uv).xyz;

		vec3 color = vec3(0.0f);
		$for (i32 i = 0, i < lights.count, ++i) {
			auto light = lights.lights[i];
			auto L = light.position - position;
			auto dist2 = max(dot(L, L), f32(1e-4f));
			auto atten = light.intensity / dist2;

			auto ldir = normalize(L);
			auto n_dot_l = max(dot(normal, ldir), f32(0.0f));

			color += (base * n_dot_l) * light.color * atten;
		};

		return color;
	};
	
	optimize(fs, flags);

	// TODO: get rid of side-effect (i.e. value based)
	// free intrinsics that are by themselves...
	assert_glsl_eq(fs, "optimization/fr_diffuse_lighting.glsl");
};

add_test(fr_diffuse_lighting_readable)
{
	auto fs = $shader(fragment)(
		$contracts(
			(lights, fwd::lights),
			(material, fwd::material)
		),
		float3 position,
		float3 normal,
		float2 uv
	) -> float3
	{
		vec3 base = material.albedo.sample(uv).xyz;

		vec3 color = vec3(0.0f);
		$for (i32 i = 0, i < lights.count, ++i) {
			auto light = lights.lights[i];
			auto L = light.position - position;
			auto dist2 = max(dot(L, L), f32(1e-4f));
			auto atten = light.intensity / dist2;

			auto ldir = normalize(L);
			auto n_dot_l = max(dot(normal, ldir), f32(0.0f));

			color += (base * n_dot_l) * light.color * atten;
		};

		return color;
	};
	
	optimize(fs, readable_flags);

	// TODO: get rid of side-effect (i.e. value based)
	// free intrinsics that are by themselves...
	assert_glsl_eq(fs, "optimization/fr_diffuse_lighting_readable.glsl");
};

add_test(ts_meshlets)
{
	auto ts = $shader(task)(
		$contracts(
			(view, meshlets::view),
			(meshlets, meshlets::meshlets)
		),
		TaskGroup <1> group
	)
	{
		TaskPayload <meshlets::TaskPayloadData> payload;

		uvec3 gid = group.workgroup_index;
		u32 meshlet_index = gid.y * view.task_group_width + gid.x;
		$if (meshlet_index < view.meshlet_count) {
			auto meshlet = meshlets[meshlet_index];
			vec3 center = vec3(meshlet.bounds);
			f32 radius = meshlet.bounds.w;

			boolean visible = true;
			$for (u32 i = 0, i < 6u, i++) {
				vec4 plane = view.frustum_planes[i];
				vec3 plane_n = vec3(plane);
				f32 dist = dot(plane_n, center) + plane.w;
				$if (dist < -radius) {
					visible = false;
				};
			};

			$if (visible) {
				payload.meshlet = meshlet_index;
				group.dispatch_mesh_groups(1u, 1u, 1u);
			};
		};

		return payload;
	};

	optimize(ts, flags);
	
	assert_glsl_eq(ts, "optimization/ts_meshlets.glsl");
};

add_test(ts_meshlets_readable)
{
	auto ts = $shader(task)(
		$contracts(
			(view, meshlets::view),
			(meshlets, meshlets::meshlets)
		),
		TaskGroup <1> group
	)
	{
		TaskPayload <meshlets::TaskPayloadData> payload;

		uvec3 gid = group.workgroup_index;
		u32 meshlet_index = gid.y * view.task_group_width + gid.x;
		$if (meshlet_index < view.meshlet_count) {
			auto meshlet = meshlets[meshlet_index];
			vec3 center = vec3(meshlet.bounds);
			f32 radius = meshlet.bounds.w;

			boolean visible = true;
			$for (u32 i = 0, i < 6u, i++) {
				vec4 plane = view.frustum_planes[i];
				vec3 plane_n = vec3(plane);
				f32 dist = dot(plane_n, center) + plane.w;
				$if (dist < -radius) {
					visible = false;
				};
			};

			$if (visible) {
				payload.meshlet = meshlet_index;
				group.dispatch_mesh_groups(1u, 1u, 1u);
			};
		};

		return payload;
	};

	optimize(ts, readable_flags);
	
	assert_glsl_eq(ts, "optimization/ts_meshlets_readable.glsl");
};

add_test(ms_meshlets)
{
	auto ms = $shader(mesh)(
		$contracts(
			(view, meshlets::view),
			(positions, meshlets::positions),
			(meshlets, meshlets::meshlets)
		),
		$contracts(
			(verts, meshlets::buffers.vertices),
			(tris, meshlets::buffers.triangles),
			(colors, meshlets::buffers.colors)
		),
		TaskPayload <meshlets::TaskPayloadData> payload,
		WorkGroup <1> group
	)
	{
		MeshletPayload <
			MeshPrimitive::eTriangles,
			64, 126,
			meshlets::MeshOutputs
		> out;

		u32 meshlet_index = payload.meshlet;
		auto meshlet = meshlets[meshlet_index];

		out.allocate(meshlet.vertex_count, meshlet.primitive_count);

		$for (u32 v = 0, v < meshlet.vertex_count, v++) {
			u32 global_index = verts[meshlet.vertex_offset + v];
			vec4 pos = positions[global_index];
			out.positions[v] = view.view_proj * pos;
			out.color[v] = colors[meshlet_index];
		};

		$for (u32 p = 0, p < meshlet.primitive_count, p++) {
			out.triangles[p] = tris[meshlet.primitive_offset + p];
		};

		return out;
	};

	optimize(ms, flags);

	assert_glsl_eq(ms, "optimization/ms_meshlets.glsl");
};

add_test(ms_meshlets_readable)
{
	auto ms = $shader(mesh)(
		$contracts(
			(view, meshlets::view),
			(positions, meshlets::positions),
			(meshlets, meshlets::meshlets)
		),
		$contracts(
			(verts, meshlets::buffers.vertices),
			(tris, meshlets::buffers.triangles),
			(colors, meshlets::buffers.colors)
		),
		TaskPayload <meshlets::TaskPayloadData> payload,
		WorkGroup <1> group
	)
	{
		MeshletPayload <
			MeshPrimitive::eTriangles,
			64, 126,
			meshlets::MeshOutputs
		> out;

		u32 meshlet_index = payload.meshlet;
		auto meshlet = meshlets[meshlet_index];

		out.allocate(meshlet.vertex_count, meshlet.primitive_count);

		$for (u32 v = 0, v < meshlet.vertex_count, v++) {
			u32 global_index = verts[meshlet.vertex_offset + v];
			vec4 pos = positions[global_index];
			out.positions[v] = view.view_proj * pos;
			out.color[v] = colors[meshlet_index];
		};

		$for (u32 p = 0, p < meshlet.primitive_count, p++) {
			out.triangles[p] = tris[meshlet.primitive_offset + p];
		};

		return out;
	};

	optimize(ms, readable_flags);

	assert_glsl_eq(ms, "optimization/ms_meshlets_readable.glsl");
};

add_test(sr_return_primitives)
{
	auto sr = $subroutine(sr)(f32 x, u32 y) {
		return std::tuple { float3(x), uint2(y, 13) };
	};

	optimize(sr, flags);
	
	assert_glsl_eq(sr, "optimization/sr_return_primitives.glsl");
};

add_test(sr_return_aggregate)
{
	auto sr = $subroutine(sr)(f32 z) {
		return Ray {
			float3(0),
			normalize(float3(1, z, 1)),
		};
	};

	optimize(sr, flags);
	
	assert_glsl_eq(sr, "optimization/sr_return_aggregate.glsl");
};

add_test(sr_invocation)
{
	auto sr1 = $subroutine(sr1)(f32 x) {
		return float3(x);
	};
	
	auto sr2 = $subroutine(sr2)(f32 x, u32 y) {
		return std::tuple { float3(x), uint2(y, 13) };
	};
	
	auto sr3 = $subroutine(sr3)(f32 z) {
		return Ray {
			float3(0),
			normalize(float3(1, z, 1)),
		};
	};

	auto vs = $shader(vertex, &)() {
		auto r1 = $use(sr1)(1);
		auto [r2a, r2b] = $use(sr2)(1.0f, 2);
		auto r3 = $use(sr3)(2.0f);
		return std::tuple { r1, r2a, r2b, r3.origin, r3.direction };
	};

	optimize(sr1, flags);
	optimize(sr2, flags);
	optimize(sr3, flags);
	optimize(vs, flags);

	assert_glsl_eq(vs, "optimization/sr_invocation.glsl");
};

add_test(sr_dependencies)
{
	auto saxpy = $subroutine(b)(f32 alpha, f32 x, f32 y) {
		return alpha * x + y;
	};

	auto saxpy2 = $subroutine(c, &)(float2 alpha, float2 x, float2 y) {
		return float2(
			$use(saxpy)(alpha.x, x.x, y.y),
			$use(saxpy)(alpha.y, x.y, y.y)
		);
	};
	
	auto saxpy3 = $subroutine(a, &)(float4 alpha, float4 x, float4 y) {
		return float4(
			$use(saxpy2)(alpha.xy, x.xy, y.yy),
			$use(saxpy2)(alpha.zw, x.zw, y.zw)
		);
	};

	auto cs = $shader(compute, &)(WorkGroup <1> group) {
		float4 x = float4(1);
		float4 y = float4(2);
		float4 alpha = float4(3);
		auto z = $use(saxpy3)(alpha, x, y);
	};

	optimize(saxpy, flags);
	optimize(saxpy2, flags);
	optimize(saxpy3, flags);
	optimize(cs, flags);

	assert_glsl_eq(cs, "optimization/sr_dependencies.glsl");
};

add_test(for_loop)
{
	auto sr = $subroutine(sr)(i32 limit, i32 step) {
		f32 sum = 0;
		$for (i32 i = 0, i < limit, i = i + step) {
			sum = sum + i;
		};

		return sum;
	};

	optimize(sr, flags);

	assert_glsl_eq(sr, "optimization/for_loop.glsl");
};

add_test(branching)
{
	auto sr = $subroutine(sr)() {
		i32 c = 12;
		$if (c > 11) {
			c += 1;
		} $elif (c < 11 and c > 5) {
			c += 2;
		} $else {
			c += 3;
		};
	};
	
	optimize(sr, flags);

	assert_glsl_eq(sr, "optimization/sr_branching.glsl");
};

add_test(rt_single_trace_group)
{
	auto rgen = $shader(raygen)(
		$contracts(
			(tlas, rt::tlas), (img, rt::image),
			(d, dispatcher <rt::radiance>)
		),
		LaunchID idx, LaunchSize size
	) {
		uint3 pixel = idx;
		d = float3(0);
		d.trace(tlas, Ray { float3(0), float3(0, 0, 1) }, f32(0.001f), f32(100.0f));
		img.write(uint2(pixel.xy), float4(d, 0));
	};

	auto chit = $shader(chit)(
		$contracts((r, receiver <rt::radiance>)),
		float2 bary, PrimitiveID prim
	) {
		r = float3(bary.x, bary.y, f32(0));
	};

	auto miss = $shader(miss)($contracts((r, receiver <rt::radiance>)))
	{
		r = float3(0);
	};

	resolve_trace_groups(rgen, std::tuple { miss }, chit);
	optimize(rgen, flags);
	optimize(chit, flags);
	optimize(miss, flags);

	assert_glsl_eq(rgen, "optimization/rt_single_rgen.glsl");
	assert_glsl_eq(chit, "optimization/rt_single_chit.glsl");
	assert_glsl_eq(miss, "optimization/rt_single_miss.glsl");
};

add_test(rt_multi_trace_group)
{
	auto rgen = $shader(raygen)(
		$contracts(
			(tlas, rt::tlas), (img, rt::image),
			(d, dispatcher <rt::radiance>)
		),
		LaunchID idx, LaunchSize size
	) {
		uint3 pixel = idx;
		d = float3(0);
		d.trace(tlas, Ray { float3(0), float3(0, 0, 1) }, f32(0.001f), f32(100.0f));
		img.write(uint2(pixel.xy), float4(d, 0));
	};

	auto chit_radiance = $shader(chit)(
		$contracts(
			(tlas, rt::tlas),
			(r, receiver <rt::radiance>),
			(shadow, dispatcher <rt::occlusion>),
			(refl, dispatcher <rt::reflection>)
		),
		float2 bary, WorldRayOrigin origin,
		WorldRayDirection direction, HitTime t
	) {
		float3 hit = float3(origin) + float3(direction) * t;

		shadow = 0.0f;
		shadow.trace(tlas, Ray { hit, float3(0, 1, 0) }, f32(0.001f), f32(100.0f));

		float3 V = normalize(float3(origin) - hit);
		float3 N = float3(0, 1, 0);
		float3 R = V - N * f32(2) * dot(V, N);
		refl = float3(0);
		refl.trace(tlas, Ray { hit, R }, f32(0.001f), f32(100.0f));

		f32 shadow_val = shadow;
		float3 refl_color = refl;
		r = float3(shadow_val) + refl_color * f32(0.3f);
	};

	auto chit_reflection = $shader(chit)(
		$contracts((r, receiver <rt::reflection>)),
		float2 bary
	) {
		r = float3(bary.x, bary.y, f32(1));
	};

	auto miss_radiance = $shader(miss)($contracts((r, receiver <rt::radiance>)))
	{ r = float3(0.5f, 0.7f, 1.0f); };

	auto miss_occlusion = $shader(miss)($contracts((s, receiver <rt::occlusion>)))
	{ s = 1.0f; };

	auto miss_reflection = $shader(miss)($contracts((r, receiver <rt::reflection>)))
	{ r = float3(0.1f, 0.1f, 0.1f); };

	resolve_trace_groups(
		rgen,
		std::tuple { miss_radiance, miss_occlusion, miss_reflection },
		chit_radiance, chit_reflection
	);

	optimize(rgen, flags);
	optimize(chit_radiance, flags);
	optimize(chit_reflection, flags);
	optimize(miss_radiance, flags);
	optimize(miss_occlusion, flags);
	optimize(miss_reflection, flags);

	assert_glsl_eq(rgen, "optimization/rt_multi_rgen.glsl");
	assert_glsl_eq(chit_radiance, "optimization/rt_multi_chit_radiance.glsl");
	assert_glsl_eq(chit_reflection, "optimization/rt_multi_chit_reflection.glsl");
	assert_glsl_eq(miss_radiance, "optimization/rt_multi_miss_radiance.glsl");
	assert_glsl_eq(miss_occlusion, "optimization/rt_multi_miss_occlusion.glsl");
	assert_glsl_eq(miss_reflection, "optimization/rt_multi_miss_reflection.glsl");
};

add_test(bufref_array)
{
	auto fs = $shader(fragment)(
		$contracts(
			(geo, bufref::geometry)
		),
		float3 position
	) -> float3
	{
		auto verts = geo[u32(0)];
		float3 p = verts.positions[u32(0)];
		float3 n = verts.normals[u32(1)];
		float2 uv = verts.uvs[u32(2)];
		return p + n + float3(uv, 0);
	};

	optimize(fs, flags);

	assert_glsl_eq(fs, "optimization/bufref_array.glsl");
};

add_test(bufref_single)
{
	auto vs = $shader(vertex)(
		$contracts(
			(scene, bufref::scene)
		),
		ClipPosition clip
	)
	{
		float4x4 xform = scene.transform;
		clip = xform * float4(0, 0, 0, 1);
	};

	optimize(vs, flags);

	assert_glsl_eq(vs, "optimization/bufref_single.glsl");
};
