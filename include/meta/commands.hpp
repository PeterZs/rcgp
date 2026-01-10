#pragma once

#include <functional>

#include <fmt/format.h>

#include "../rhi/command_buffer.hpp"
#include "../util/timer.hpp"
#include "pipeline/rasterization.hpp"
#include "vertex_buffer_for.hpp"

// Auxiliary command buffer state as dependency information
// TODO: concept for command state

struct CommandsTraceAux {
	vk::PipelineBindPoint bind_point;
	vk::PipelineLayout layout;
	// TODO: pipeline handle -> (layout, maps)
	// TODO: also track pipeline stage for syncing
};

// TODO: more efficient mechanism for this?
using command_operator = std::function <void (const CommandBuffer &, CommandsTraceAux &)>;

template <typename ... Effects>
struct Commands : std::vector <command_operator> {
	using std::vector <command_operator> ::vector;

	void trace(const CommandBuffer &cmd, CommandsTraceAux &aux) const {
		for (auto &op : *this)
			op(cmd, aux);
	}

	auto &operator()(const vk::CommandBuffer &cmd) const {
		TSCOPE("commands serialization");

		CommandsTraceAux aux;

		cmd.reset();
		cmd.begin(vk::CommandBufferBeginInfo());

		TimerToken::note(fmt::format("{} commands traced", size()));
		trace(cmd, aux);

		cmd.end();

		return cmd;
	}
};

TYPE_TRAIT(is_commands);
	template <typename ... Effects>
	TYPE_TRAIT_INCLUDES(is_commands, Commands <Effects...>);

template <typename ... EAs, typename ... EBs>
auto cmdcat(const Commands <EAs...> &x, const Commands <EBs...> &y)
{
	// TODO: simplify EAs + EBs
	auto cmd = Commands <EAs..., EBs...> {};
	cmd.append_range(x);
	cmd.append_range(y);
	return cmd;
}

template <typename First, typename Second, typename ...Rest>
auto cmdcat(const First &first, const Second &second, const Rest &... rest)
{
	static_assert(sizeof...(Rest) >= 0, "seq requires at least two arguments");
	auto combined = cmdcat(first, second);
	if constexpr (sizeof...(Rest) == 0)
		return combined;
	else
		return cmdcat(combined, rest...);
}

// Pipe operator: syntactic sugar for seq
template <typename ... EAs, typename ... EBs>
auto operator|(const Commands <EAs...> &lhs, const Commands <EBs...> &rhs)
{
	return cmdcat(lhs, rhs);
}

template <typename ... EAs>
auto operator|(const Commands <EAs...> &x, const std::nullptr_t &)
{
	return x;
}

template <typename ... EAs>
auto operator|(const std::nullptr_t &, const Commands <EAs...> &x)
{
	return x;
}

inline auto begin_render_pass(const vk::RenderPass &render_pass,
		       const vk::Framebuffer &framebuffer,
		       const vk::Rect2D &render_area,
		       const std::span <vk::ClearValue> &clear_values)
{
	auto binder = [=](const CommandBuffer &cmd, CommandsTraceAux &) {
		auto rp_begin = vk::RenderPassBeginInfo()
			.setRenderPass(render_pass)
			.setFramebuffer(framebuffer)
			.setRenderArea(render_area)
			.setClearValues(clear_values);

		cmd.beginRenderPass(rp_begin, vk::SubpassContents::eInline);
	};

	return Commands <> { binder };
}

template <Topology T, typename AttributeStreams, typename GroupAllocation, typename GlobalResources>
auto bind_pipeline(const AnnotatedRasterizationPipeline <T, AttributeStreams, GroupAllocation, GlobalResources> &pipeline)
{
	// TODO: write bind point and layout in an interm state (CommandBufferAux &)?
	auto binder = [=](const CommandBuffer &cmd, CommandsTraceAux &aux) {
		aux.bind_point = vk::PipelineBindPoint::eGraphics;
		aux.layout = pipeline.layout;
		cmd.bindPipeline(aux.bind_point, pipeline.handle);
	};

	return Commands <> { binder };
}

template <auto &... refs>
auto bind_descriptors(const DescriptorFor <refs, true> &... descriptors)
{
	auto binder = [=](const CommandBuffer &cmd, CommandsTraceAux &aux) {
		(cmd.bindDescriptorSets(
			aux.bind_point,
			aux.layout,
			descriptors.set, { descriptors.handle }, {}
		), ...);
	};

	return Commands <> { binder };
}

template <auto &ref, Topology T, typename AttributeStreams, typename GroupAllocation, typename GlobalResources>
auto bind_push_constants(const AnnotatedRasterizationPipeline <T, AttributeStreams, GroupAllocation, GlobalResources> &,
		    const ResourceTypeFor <ref> &constants)
{
	static_assert(is_push_constant_v <reference_base_t <ref>>);

	constexpr auto stage_flags = stage_flags_for_v <ref, GlobalResources>;
	static_assert(stage_flags != vk::ShaderStageFlags(), "push constant not used by any stage");
	static_assert(push_constant_offset_found_v <ref, GlobalResources>,
		"push constant not found in pipeline layout");
	static_assert(sizeof(ResourceTypeFor <ref>) % 4u == 0u,
		"push constant size must be a multiple of 4 bytes");

	auto binder = [stage_flags, constants](const CommandBuffer &cmd, CommandsTraceAux &aux) {
		constexpr auto offset = push_constant_offset_for_v <ref, GlobalResources>;
		cmd.pushConstants <ResourceTypeFor <ref>> (aux.layout, stage_flags, offset, constants);
	};

	return Commands <> { binder };
}

// TODO: bind_vertex_buffer only form the pipeline
template <auto &ref>
auto bind_vertex_buffer(const VertexBufferFor <ref> &buffer, size_t boffset)
{
	auto binder = [=](const CommandBuffer &cmd, CommandsTraceAux &) {
		cmd.bindVertexBuffers(boffset, { buffer.handle }, { 0 });
	};

	return Commands <> { binder };
}

// template <auto &... refs>
// auto bind_vertex_buffers(const VertexBufferOf <refs> &... buffers)
// {
// 	auto binder = [=](const vk::CommandBuffer &cmd, CommandsTraceAux &) {
// 		std::array <vk::DeviceSize, sizeof...(refs)> offsets {};
// 		cmd.bindVertexBuffers(0, { buffers.handle... }, offsets);
// 	};
//
// 	return Commands <> { binder };
// }

// and add a state tag that encodes this... ProvidedIndexBuffer <T>
template <Topology T, typename I>
inline auto bind_index_buffer(const IndexBuffer <T, I> &ibuffer)
{
	// TODO: templates for inferring the index type
	auto binder = [=](const CommandBuffer &cmd, CommandsTraceAux &) {
		cmd.bindIndexBuffer(ibuffer.handle, 0, vk::IndexType::eUint32);
	};

	return Commands <> { binder };
}

inline auto draw_indexed(uint32_t count)
{
	// TODO: should return an aux state which adds back the dependencies
	auto binder = [=](const CommandBuffer &cmd, CommandsTraceAux &) {
		cmd.drawIndexed(count, 1, 0, 0, 0);
	};

	return Commands <> { binder };
}

inline auto end_render_pass()
{
	auto binder = [](const CommandBuffer &cmd, CommandsTraceAux &) {
		cmd.endRenderPass();
	};

	return Commands <> { binder };
}

inline auto manual_commands(auto F)
{
	return Commands <> { F };
}

template <typename T, typename F>
requires is_commands_v <std::invoke_result_t <F, T>>
inline auto foreach(const std::vector <T> &container, F &&ftn)
{
	using C = std::invoke_result_t <F, T>;

	auto binder = [&](const CommandBuffer &cmd, CommandsTraceAux &aux) {
		for (auto &&value : container)
			ftn(value).trace(cmd, aux);
	};

	return C { binder };
}
