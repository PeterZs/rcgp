#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "instructions.hpp"

namespace rcgp {

using reference_allocation_map = std::map <void *, uint32_t>;

// TODO: should transition to CFG style...
// CFG nodes and a top level Module which include context info
struct Block : std::vector <Reference> {
	ShaderStage stage = ShaderStage::eSubroutine;
	std::map <uint32_t, bool> mesh_perprimitive_outputs;
	std::map <void *, std::vector <Reference>> global_resources;
	std::map <void *, std::vector <Reference>> trace_groups;
	std::optional <MeshPrimitive> mesh_primitive_kind;
	std::optional <Reference> hit_attribute_type;
	std::optional <Reference> task_payload_type;
	std::optional <std::array <uint32_t, 3>> workgroup_size;
	std::optional <uint32_t> mesh_max_primitives;
	std::optional <uint32_t> mesh_max_vertices;
	std::string name;
	std::vector <Argument> arguments;
	std::vector <Return> returns;
	std::vector <StageInput> stage_inputs;
	std::vector <StageOutput> stage_outputs;
	uint32_t mesh_output_counter = 0;
	
	void add_argument(const Argument &arg);
	void add_return(const Return &ret);
	void add_stage_input(const StageInput &tin);
	void add_stage_output(const StageOutput &tout);
	void add_global_resource(void *addr, const Reference &resource);
	void add_trace_group(void *addr, const Reference &trace_call);
	void set_workgroup_size(uint32_t x, uint32_t y, uint32_t z);
	void apply_group_allocation_map(const reference_allocation_map &map);
	void apply_push_constant_allocation_map(const reference_allocation_map &map);
	void apply_ray_payload_allocation_map(const reference_allocation_map &map);

	Reference add(const Instruction &instr);
	Reference add(size_t index, const Instruction &instr);

	std::string repr() const;
};

template <typename F>
void walk_instructions(const SharedBlockReference &sbr, F &&fn, bool walk_invocations = false)
{
	for (auto &instr : *sbr) {
		fn(instr);
		if (auto branch = instr->maybe <Branch> ()) {
			for (auto &seg : branch->segments)
				walk_instructions(seg.body, fn, walk_invocations);
			if (branch->fallback)
				walk_instructions(*branch->fallback, fn, walk_invocations);
		} else if (auto loop = instr->maybe <Loop> ()) {
			walk_instructions(loop->body, fn, walk_invocations);
		} else if (walk_invocations) {
			if (auto inv = instr->maybe <Invocation> ())
				walk_instructions(inv->sbr, fn, walk_invocations);
		}
	}
}

template <typename F>
void walk_blocks(const SharedBlockReference &sbr, F &&fn, bool walk_invocations = false)
{
	fn(sbr);
	for (auto &instr : *sbr) {
		if (auto branch = instr->maybe <Branch> ()) {
			for (auto &seg : branch->segments)
				walk_blocks(seg.body, fn, walk_invocations);
			if (branch->fallback)
				walk_blocks(*branch->fallback, fn, walk_invocations);
		} else if (auto loop = instr->maybe <Loop> ()) {
			walk_blocks(loop->body, fn, walk_invocations);
		} else if (walk_invocations) {
			if (auto inv = instr->maybe <Invocation> ())
				walk_blocks(inv->sbr, fn, walk_invocations);
		}
	}
}

} // namespace rcgp
