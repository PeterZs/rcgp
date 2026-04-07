#include "dsl/block.hpp"
#include "dsl/instructions.hpp"
#include "util/error.hpp"

namespace rcgp {

void Block::apply_group_allocation_map(const reference_allocation_map &map)
{
	for (auto &[addr, group] : map) {
		if (global_resources.contains(addr)) {
			for (auto &ref : global_resources.at(addr))
				ref->as <GlobalResource> ().group = group;
		}
	}

	// Apply recursively to invoked blocks
	for (auto &instr : *this) {
		if (instr->is <Invocation> ()) {
			auto sbr = instr->as <Invocation> ().sbr;
			sbr->apply_group_allocation_map(map);
		}
	}
}

void Block::apply_push_constant_allocation_map(const reference_allocation_map &map)
{
	for (auto &[addr, offset] : map) {
		if (global_resources.contains(addr)) {
			for (auto &ref : global_resources.at(addr))
				ref->as <GlobalResource> ().offset = offset;
		}
	}
}

void Block::apply_ray_payload_allocation_map(const reference_allocation_map &map)
{
	for (auto &[addr, index] : map) {
		if (global_resources.contains(addr)) {
			for (auto &ref : global_resources.at(addr)) {
				auto &grsrc = ref->as <GlobalResource> ();
				if (grsrc.kind == GlobalResourceKind::eRayDispatcherPayload
					or grsrc.kind == GlobalResourceKind::eRayReceiverPayload)
					grsrc.index = index;
			}
		}
	}
}

void Block::add_argument(const Argument &arg)
{
	if (arguments.size() > arg.argi) {
		fatal("argument {} already registered", arg.argi);
	} else {
		arguments.resize(arg.argi + 1);
		arguments[arg.argi] = arg;
	}
}

void Block::add_return(const Return &ret)
{
	if (returns.size() > ret.argi) {
		fatal("return {} already registered", ret.argi);
	} else {
		returns.resize(ret.argi + 1);
		returns[ret.argi] = ret;
	}
}

void Block::add_stage_input(const StageInput &sin)
{
	if (stage_inputs.size() > sin.argi) {
		fatal("stage input {} already registered", sin.argi);
	} else {
		stage_inputs.resize(sin.argi + 1);
		stage_inputs[sin.argi] = sin;
	}
}

void Block::add_stage_output(const StageOutput &sout)
{
	if (stage_outputs.size() > sout.argi) {
		// TODO: this is fine, just make sure its the same or
		// its uninitialized...
		fatal("stage output {} already registered", sout.argi);
	} else {
		stage_outputs.resize(sout.argi + 1);
		stage_outputs[sout.argi] = sout;
	}
}

void Block::add_global_resource(void *addr, const Reference &resource)
{
	global_resources[addr].push_back(resource);
}

void Block::add_trace_group(void *addr, const Reference &trace_call)
{
	trace_groups[addr].push_back(trace_call);
}

void Block::set_workgroup_size(uint32_t x, uint32_t y, uint32_t z)
{
	auto size = std::array <uint32_t, 3> { x, y, z };
	if (workgroup_size.has_value() && workgroup_size.value() != size)
		fatal("workgroup size mismatch");
	workgroup_size = size;
}

Reference Block::add(const Instruction &instr)
{
	auto ref = std::make_shared <Instruction> (instr);
	push_back(ref);
	return ref;
}

Reference Block::add(size_t index, const Instruction &instr)
{
	auto ref = std::make_shared <Instruction> (instr);
	insert(begin() + index, ref);
	return ref;
}

} // namespace rcgp
