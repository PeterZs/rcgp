#pragma once

#include <map>
#include <vector>

#include "../dsl/block.hpp"
#include "../dsl/instructions.hpp"
#include "../util/error.hpp"

namespace rcgp {

struct trace_group_layout {
	uint32_t group_count;
	std::vector <uint32_t> chit_to_group;
};

template <typename ... MissShaders>
auto resolve_trace_groups(
	const SharedBlockReference &rgen,
	const std::tuple <MissShaders...> &misses,
	auto &... chit_blocks
) -> trace_group_layout
{
	std::map <void *, uint32_t> miss_map;

	auto register_miss = [&] <size_t... Is> (std::index_sequence <Is...>) {
		auto process = [&](auto &miss_shader, uint32_t index) {
			for (auto &[addr, refs] : miss_shader->global_resources) {
				for (auto &ref : refs) {
					if (not ref->template is <GlobalResource> ())
						continue;
					auto &grsrc = ref->template as <GlobalResource> ();
					if (grsrc.kind == GlobalResourceKind::eRayReceiverPayload)
						miss_map.emplace(addr, index);
				}
			}
		};

		(process(std::get <Is> (misses), uint32_t(Is)), ...);
	};

	register_miss(std::index_sequence_for <MissShaders...> {});

	const uint32_t G = uint32_t(miss_map.size());

	std::map <void *, uint32_t> payload_map;
	uint32_t next_payload = 0;

	auto assign_payload = [&](void *addr) {
		if (not payload_map.contains(addr))
			payload_map.emplace(addr, next_payload++);
	};

	for (auto &[addr, _] : rgen->trace_groups)
		assign_payload(addr);

	for (auto &[addr, _] : miss_map)
		assign_payload(addr);

	auto assign_from_chit = [&](auto &block) {
		for (auto &[addr, refs] : block->global_resources) {
			for (auto &ref : refs) {
				if (not ref->template is <GlobalResource> ())
					continue;
				auto &grsrc = ref->template as <GlobalResource> ();
				if (grsrc.kind == GlobalResourceKind::eRayReceiverPayload)
					assign_payload(addr);
			}
		}
	};

	(assign_from_chit(chit_blocks), ...);

	auto group_of_chit = [&](auto &block) -> uint32_t {
		for (auto &[addr, refs] : block->global_resources) {
			for (auto &ref : refs) {
				if (not ref->template is <GlobalResource> ())
					continue;
				auto &grsrc = ref->template as <GlobalResource> ();
				if (grsrc.kind != GlobalResourceKind::eRayReceiverPayload)
					continue;
				auto it = miss_map.find(addr);
				if (it != miss_map.end())
					return it->second;
			}
		}
		return 0;
	};

	std::vector <uint32_t> chit_to_group;
	chit_to_group.reserve(sizeof...(chit_blocks));
	(chit_to_group.push_back(group_of_chit(chit_blocks)), ...);

	auto resolve_block = [&](this auto &self, const SharedBlockReference &block) -> void {
		for (auto &[addr, trace_calls] : block->trace_groups) {
			auto miss_it = miss_map.find(addr);
			assertion(miss_it != miss_map.end(), "missing miss shader for trace group");

			auto payload_it = payload_map.find(addr);
			assertion(payload_it != payload_map.end(), "missing payload for trace group");

			for (auto &ref : trace_calls) {
				auto &bintr = ref->as <BuiltinIntrinsic> ();
				assertion(bintr.code == BuiltinIntrinsicCode::eTraceRaysEXT, "expected TraceRaysEXT intrinsic");
				assertion(bintr.args.size() == 11, "TraceRaysEXT expects 11 arguments");

				bintr.args[3] = std::make_shared <Instruction> (
					Constant(int32_t(miss_it->second))
				);

				bintr.args[4] = std::make_shared <Instruction> (
					Constant(int32_t(G))
				);

				bintr.args[5] = std::make_shared <Instruction> (
					Constant(int32_t(miss_it->second))
				);

				bintr.args[10] = std::make_shared <Instruction> (
					Constant(int32_t(payload_it->second))
				);
			}
		}

		for (auto &ref : *block) {
			if (ref->is <Branch> ()) {
				auto &branch = ref->as <Branch> ();
				for (auto &seg : branch.segments)
					self(seg.body);
				if (branch.fallback)
					self(*branch.fallback);
			} else if (ref->is <Loop> ()) {
				self(ref->as <Loop> ().body);
			}
		}
	};

	resolve_block(rgen);
	(resolve_block(chit_blocks), ...);

	rgen->apply_ray_payload_allocation_map(payload_map);
	[&] <size_t... Is> (std::index_sequence <Is...>) {
		(std::get <Is> (misses)->apply_ray_payload_allocation_map(payload_map), ...);
	}(std::index_sequence_for <MissShaders...> {});
	(chit_blocks->apply_ray_payload_allocation_map(payload_map), ...);

	return { G, std::move(chit_to_group) };
}

} // namespace rcgp
