#include "common.hpp"
#include "common_resources.hpp"

#define SUITE "pipelines"

// Pipelines:
// Testing that combinators produce valid Vulkan pipelines from shader
// combinations. Requires a live Vulkan device.

static ShaderCompiler compiler;

static auto &device()
{
	static auto dev = [] {
		auto features = vk::StructureChain <
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan12Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
			vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
			vk::PhysicalDeviceMeshShaderFeaturesEXT
		> ();

		features.get <vk::PhysicalDeviceVulkan12Features> ()
			.setScalarBlockLayout(true)
			.setBufferDeviceAddress(true)
			.setDescriptorIndexing(true)
			.setRuntimeDescriptorArray(true)
			.setDescriptorBindingVariableDescriptorCount(true);

		features.get <vk::PhysicalDeviceVulkan13Features> ()
			.setMaintenance4(true)
			.setSynchronization2(true)
			.setDynamicRendering(true);

		features.get <vk::PhysicalDeviceRayTracingPipelineFeaturesKHR> ()
			.setRayTracingPipeline(true);

		features.get <vk::PhysicalDeviceAccelerationStructureFeaturesKHR> ()
			.setAccelerationStructure(true);

		features.get <vk::PhysicalDeviceMeshShaderFeaturesEXT> ()
			.setMeshShader(true)
			.setTaskShader(true);

		auto [session, dld] = Session::from({
			.application_name = "rcgp-tests",
			.engine_name = "rcgp",
			.validation = false,
		});

		return Device::from(session, dld, {
			.extensions = {
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_EXT_MESH_SHADER_EXTENSION_NAME,
			},
			.queues = {
				{ "graphics", vk::QueueFlagBits::eGraphics },
			},
			.pNext = &features.get <vk::PhysicalDeviceFeatures2> (),
		});
	} ();

	return dev;
}

static auto render_state(vk::Format color = vk::Format::eB8G8R8A8Unorm)
{
	return vk::PipelineRenderingCreateInfo()
		.setColorAttachmentFormats(color);
}

static auto depth_render_state(
	vk::Format color = vk::Format::eB8G8R8A8Unorm,
	vk::Format depth = vk::Format::eD32Sfloat
)
{
	return vk::PipelineRenderingCreateInfo()
		.setColorAttachmentFormats(color)
		.setDepthAttachmentFormat(depth);
}

// Rasterization
add_test(raster_passthrough)
{
	auto vs = $shader(vertex)(ClipPosition clip) {
		clip = float4(0.0f);
		return Smooth(float3(1.0f));
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto rs = render_state();
	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);
};

add_test(raster_vertex_only)
{
	auto vs = $shader(vertex)(ClipPosition clip) { clip = float4(0.0f); };

	auto rs = depth_render_state();
	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (vs);
};

add_test(raster_multi_io)
{
	auto vs = $shader(vertex)(
		$contracts(
			(pos, fwd::position),
			(nrm, fwd::normal),
			(uv, fwd::uv)
		),
		ClipPosition clip
	) {
		clip = float4(pos, 1.0f);
		return std::tuple {
			Smooth <float3> { pos },
			Smooth <float3> { nrm },
			Smooth <float2> { uv },
		};
	};
	auto fs = $shader(fragment)(float3 position, float3 normal, float2 uv) -> float3 {
		return position + normal + float3(uv, 0);
	};

	auto rs = render_state();
	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);
};

add_test(raster_push_constant)
{
	auto vs = $shader(vertex)(
		$contracts(
			(view, fwd::view),
			(pos, fwd::position),
			(nrm, fwd::normal),
			(uv, fwd::uv)
		),
		ClipPosition cpos
	)
	{
		float4 wpos = view.model * float4(pos, 1);
		cpos = view.proj * view.view * wpos;
		return std::tuple {
			Smooth <float3> { float3(wpos) },
			Smooth <float3> { nrm },
			Smooth <float2> { uv },
		};
	};
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
		$for (i32 i = 0, i < lights.count, i++) {
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

	auto rs = render_state();
	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);
};

add_test(raster_with_subroutine)
{
	auto lighting = $subroutine(lighting)(float3 normal, float3 light_dir) {
		return max(dot(normal, light_dir), f32(0.0f));
	};

	auto vs = $shader(vertex, &)(ClipPosition clip) {
		clip = float4(0.0f);
		auto n_dot_l = $use(lighting)(float3(0, 1, 0), float3(0, 1, 0));
		return Smooth(float3(n_dot_l));
	};

	auto fs = $shader(fragment, &)(float3 color) -> float4 {
		auto n_dot_l = $use(lighting)(float3(0, 0, 1), float3(0, 1, 0));
		return float4(color * n_dot_l, 1.0f);
	};

	auto rs = render_state();
	auto pipeline = RasterizationCombinator <Topology::eTriangleList> {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (vs, fs);
};

// Compute
add_test(compute_empty)
{
	auto cs = $shader(compute)(WorkGroup <1> group) {};

	auto pipeline = ComputeCombinator {
		.device = device(),
		.compiler = compiler,
	} (cs);
};

add_test(compute_storage_buffer)
{
	auto cs = $shader(compute)(
		$contracts((positions, meshlets::positions)),
		WorkGroup <64> group
	) {
		float4 p = positions[u32(0)];
	};

	auto pipeline = ComputeCombinator {
		.device = device(),
		.compiler = compiler,
	} (cs);
};

add_test(compute_with_subroutine)
{
	auto saxpy = $subroutine(saxpy)(f32 alpha, f32 x, f32 y) {
		return alpha * x + y;
	};

	auto cs = $shader(compute, &)(WorkGroup <1> group) {
		auto z = $use(saxpy)(f32(2), f32(3), f32(4));
	};

	auto pipeline = ComputeCombinator {
		.device = device(),
		.compiler = compiler,
	} (cs);
};

// Mesh shading
add_test(mesh_pipeline_minimal)
{
	auto ts = $shader(task)(TaskGroup <1> group) {
		TaskPayload <float3> payload;
		group.dispatch_mesh_groups(1u, 1u, 1u);
		return payload;
	};
	auto ms = $shader(mesh)(TaskPayload <float3> payload, WorkGroup <1> group) {
		MeshletPayload <MeshPrimitive::eTriangles, 64, 126, meshlets::MeshOutputs> mp;
		return mp;
	};
	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto rs = render_state();
	auto pipeline = MeshShadingCombinator {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (ts, ms, fs);
};

add_test(mesh_pipeline_meshlets)
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

	auto fs = $shader(fragment)(float3 color) { return float4(color, 1.0f); };

	auto rs = render_state();
	auto pipeline = MeshShadingCombinator {
		.device = device(),
		.compiler = compiler,
		.render_state = rs,
	} (ts, ms, fs);
};

// Raytracing
add_test(rt_single_trace_group)
{
	auto rgen = $shader(raygen)(
		$contracts(
			(tlas, rt::tlas), (img, rt::image),
			(radiance, dispatcher <rt::radiance>)
		),
		LaunchID idx, LaunchSize size
	) {
		uint3 pixel = idx;
		radiance = float3(0);
		radiance.trace(tlas, Ray { float3(0), float3(0, 0, 1) }, f32(0.001f), f32(100.0f));
		img.write(uint2(pixel.xy), float4(radiance, 0));
	};

	auto chit = $shader(chit)(
		$contracts((radiance, receiver <rt::radiance>)),
		float2 bary, PrimitiveID prim
	) {
		radiance = float3(bary.x, bary.y, f32(0));
	};

	auto miss = $shader(miss)($contracts((radiance, receiver <rt::radiance>)))
	{
		radiance = float3(0);
	};

	auto pipeline = RayTracingCombinator {
		.device = device(),
		.compiler = compiler,
	} (rgen, std::tuple { miss }, std::tuple { chit });
};

add_test(rt_multi_trace_group)
{
	auto rgen = $shader(raygen)(
		$contracts(
			(tlas, rt::tlas), (img, rt::image),
			(radiance, dispatcher <rt::radiance>)
		),
		LaunchID idx, LaunchSize size
	) {
		uint3 pixel = idx;
		radiance = float3(0);
		radiance.trace(tlas, Ray { float3(0), float3(0, 0, 1) }, f32(0.001f), f32(100.0f));
		img.write(uint2(pixel.xy), float4(radiance, 0));
	};

	auto chit_radiance = $shader(chit)(
		$contracts(
			(tlas, rt::tlas),
			(radiance, receiver <rt::radiance>),
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
		radiance = float3(shadow_val) + refl_color * f32(0.3f);
	};

	auto chit_reflection = $shader(chit)(
		$contracts((radiance, receiver <rt::reflection>)),
		float2 bary
	) {
		radiance = float3(bary.x, bary.y, f32(1));
	};

	auto miss_radiance = $shader(miss)($contracts((r, receiver <rt::radiance>)))
	{ r = float3(0.5f, 0.7f, 1.0f); };

	auto miss_occlusion = $shader(miss)($contracts((s, receiver <rt::occlusion>)))
	{ s = 1.0f; };

	auto miss_reflection = $shader(miss)($contracts((r, receiver <rt::reflection>)))
	{ r = float3(0.1f, 0.1f, 0.1f); };

	auto pipeline = RayTracingCombinator {
		.device = device(),
		.compiler = compiler,
	} (rgen,
		std::tuple { miss_radiance, miss_occlusion, miss_reflection },
		std::tuple { chit_radiance, chit_reflection }
	);
};
