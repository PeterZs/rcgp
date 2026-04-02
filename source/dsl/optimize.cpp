#include <map>
#include <set>
#include <algorithm>

#include "dsl/block.hpp"
#include "dsl/instructions.hpp"
#include "dsl/optimization.hpp"
#include "util/error.hpp"
#include "optimize_type.cpp"

namespace rcgp {

struct InstructionSet {
	std::set <Reference> accounted;
	std::vector <Reference> ordered;

	void add(const Reference &ref) {
		if (not accounted.contains(ref)) {
			accounted.insert(ref);
			ordered.emplace_back(ref);
		}
	}
};

using InstructionMap = std::map <Reference, InstructionSet>;

// TODO: persistent sbr structure that is updated instead of rebuilt
struct WholeBlockStructure {
	InstructionMap i2o;
	InstructionMap o2i;
	std::map <Reference, SharedBlockReference> blocks;
};

void build_usage_map(
	const SharedBlockReference &sbr,
	WholeBlockStructure &wbs
);

void add_to_usage_maps(
	const Reference &ref,
	WholeBlockStructure &wbs
)
{
	auto &i2o = wbs.i2o;
	auto &o2i = wbs.o2i;
	auto &operands = i2o.try_emplace(ref).first->second;

	auto add = [&](const Reference &opd) {
		if (not opd) return;
		operands.add(opd);
		auto &users = o2i.try_emplace(opd).first->second;
		users.add(ref);
	};

	ref->apply(add);

	// Nested blocks
	if (auto branch = ref->maybe <Branch> ()) {
		for (auto &b : branch->segments)
			build_usage_map(b.body, wbs);
		if (branch->fallback)
			build_usage_map(*branch->fallback, wbs);
	} else if (auto loop = ref->maybe <Loop> ()) {
		build_usage_map(loop->body, wbs);
	}
}

void build_usage_map(
	const SharedBlockReference &sbr,
	WholeBlockStructure &wbs
)
{
	auto &i2o = wbs.i2o;
	auto &o2i = wbs.o2i;
	for (auto &instr : *sbr) {
		wbs.blocks.try_emplace(instr, sbr);
		i2o.try_emplace(instr);
		o2i.try_emplace(instr);
		add_to_usage_maps(instr, wbs);
	}
}

auto build_whole_block_structure(const SharedBlockReference &sbr)
{
	WholeBlockStructure result;
	build_usage_map(sbr, result);
	return result;
}

void collect_blocks(
	const SharedBlockReference &sbr,
	std::vector <SharedBlockReference> &sbrs
)
{
	sbrs.emplace_back(sbr);
	for (auto &instr : *sbr) {
		if (auto branch = instr->maybe <Branch> ()) {
			for (auto &b : branch->segments)
				collect_blocks(b.body, sbrs);
			if (branch->fallback)
				collect_blocks(*branch->fallback, sbrs);
		} else if (auto loop = instr->maybe <Loop> ()) {
			collect_blocks(loop->body, sbrs);
		}
	}
}

// TODO: collect sbrs

bool dead_code_elimination_pass(const SharedBlockReference &sbr)
{
	auto wbs = build_whole_block_structure(sbr);

	std::set <Reference> remove;
	for (auto &[instr, uses] : wbs.o2i) {
		if (instr->is <Store> ()) {
			auto &store = instr->as <Store> ();
			auto &dst = store.destination;
			if (dst->is <Local> ()) {
				auto &uses = wbs.o2i.at(dst);
				if (uses.ordered.size() == 1)
					remove.insert(instr);
			}
		} else if (instr->is <Local> ()) {
			if (uses.ordered.empty())
				remove.insert(instr);
		}
	}
	
	std::vector <SharedBlockReference> blocks;
	collect_blocks(sbr, blocks);
	
	bool changed = false;
	for (auto &sbr : blocks) {
		changed |= std::erase_if(*sbr, [&](auto instr) {
			return remove.contains(instr);
		});
	}

	return changed;
}

bool local_store_elide(
	const WholeBlockStructure &wbs,
	const Reference &ref,
	const std::vector <Reference> &uses
)
{
	if (not ref->is <Local> ())
		return false;

	auto &local = ref->as <Local> ();
	if (local.init) {
		if (local.init == ref)
			return false;

		auto init_uses_it = wbs.o2i.find(local.init);
		if (init_uses_it == wbs.o2i.end())
			return false;

		auto &init_uses = init_uses_it->second;
		if (init_uses.ordered.size() != 1)
			return false;
		if (not init_uses.accounted.contains(ref))
			return false;

		std::vector <Reference> reads;
		for (auto &user : uses) {
			if (auto store = user->maybe <Store> ()) {
				if (store->destination == ref)
					return false;
			}

			reads.emplace_back(user);
		}

		size_t occurrences = 0;
		for (auto &user : reads) {
			user->apply([&](Reference &opd) {
				if (opd == ref)
					occurrences++;
			});
		}

		if (occurrences != 1)
			return false;

		auto value = local.init;
		bool changed = false;
		for (auto &user : reads) {
			user->apply([&](Reference &opd) {
				if (opd == ref) {
					opd = value;
					changed |= true;
				}
			});
		}
		return changed;
	}

	if (uses.size() < 2)
		return false;

	auto first = uses[0];
	if (not first->is <Store> ())
		return false;

	auto fstore = first->as <Store> ();
	if (fstore.destination != ref)
		return false;

	// Find the second store (if any)
	Reference second = nullptr;
	for (size_t i = 1; i < uses.size(); i++) {
		auto &user = uses[i];
		if (not user->is <Store> ())
			continue;

		auto &store = user->as <Store> ();
		if (store.destination == ref) {
			second = user;
			break;
		}
	}

	// For now only deal with useless locals
	if (second)
		return false;

	// TODO: may have to be careful if the source value changes across instructions?
	auto value = fstore.source;

	bool changed = false;
	for (size_t i = 1; i < uses.size(); i++) {
		auto &user = uses[i];
		user->apply([&](Reference &opd) {
			if (opd == ref) {
				opd = value;
				changed |= true;
			}
		});
	}

	return changed;
}

bool local_elision_pass(const SharedBlockReference &sbr)
{
	auto wbs = build_whole_block_structure(sbr);
	
	bool changed = false;
	for (auto &[instr, uses] : wbs.o2i)
		changed |= local_store_elide(wbs, instr, uses.ordered);
	return changed;
}

bool has_only_local_init_uses(const Reference &ref, const std::vector <Reference> &uses)
{
	if (uses.empty())
		return false;

	for (auto &user : uses) {
		if (not user->is <Local> ())
			return false;

		auto &local = user->as <Local> ();
		if (local.init != ref)
			return false;
	}
	return true;
}

bool has_side_effects(const BuiltinIntrinsicCode code)
{
	switch (code) {
	case BuiltinIntrinsicCode::eBreak:
	case BuiltinIntrinsicCode::eContinue:
	case BuiltinIntrinsicCode::eDiscard:
	case BuiltinIntrinsicCode::eEmitMeshTasksEXT:
	case BuiltinIntrinsicCode::eSetMeshOutputsEXT:
		return true;
	default:
		break;
	}

	return false;
}

bool is_promotable_expression(const Reference &ref)
{
	vswitch (*ref) {
	vcase(Argument):
	vcase(ArrayAccess):
	vcase(Constant):
	vcase(Construct):
	vcase(FieldAccess):
	vcase(GlobalResource):
	vcase(Local):
	vcase(Operation):
	vcase(Return):
	vcase(StageInput):
	vcase(StageOutput):
	vcase(Swizzle):
	vcase(SystemValue): {
		return true;
	}
	vcase(BuiltinIntrinsic): {
		return not has_side_effects(ref->as <BuiltinIntrinsic> ().code);
	}
	default:
		break;
	}

	return false;
}

size_t expression_tree_size(
	const Reference &ref,
	std::map <Reference, size_t> &memo
)
{
	if (not ref)
		return 0;

	if (memo.contains(ref))
		return memo.at(ref);

	size_t size = 1;
	vswitch (*ref) {
	vcase(ArrayAccess): {
		auto &aacc = ref->as <ArrayAccess> ();
		size += expression_tree_size(aacc.value, memo);
		size += expression_tree_size(aacc.index, memo);
		break;
	}
	vcase(BuiltinIntrinsic): {
		auto &bintr = ref->as <BuiltinIntrinsic> ();
		for (auto &arg : bintr.args)
			size += expression_tree_size(arg, memo);
		break;
	}
	vcase(Construct): {
		auto &ctor = ref->as <Construct> ();
		for (auto &arg : ctor.args)
			size += expression_tree_size(arg, memo);
		break;
	}
	vcase(FieldAccess): {
		auto &facc = ref->as <FieldAccess> ();
		size += expression_tree_size(facc.value, memo);
		break;
	}
	vcase(Operation): {
		auto &opn = ref->as <Operation> ();
		size += expression_tree_size(opn.a, memo);
		size += expression_tree_size(opn.b, memo);
		break;
	}
	vcase(Swizzle): {
		auto &swz = ref->as <Swizzle> ();
		size += expression_tree_size(swz.value, memo);
		break;
	}
	default:
		break;
	}

	memo.emplace(ref, size);
	return size;
}

template <typename F>
void for_each_expression_operand(const Reference &ref, F &&ftn)
{
	auto &&fn = ftn;
	vswitch (*ref) {
	vcase(ArrayAccess): {
		auto &aacc = ref->as <ArrayAccess> ();
		fn(aacc.value);
		fn(aacc.index);
		break;
	}
	vcase(BuiltinIntrinsic): {
		auto &bintr = ref->as <BuiltinIntrinsic> ();
		for (auto &arg : bintr.args)
			fn(arg);
		break;
	}
	vcase(Branch): {
		auto &branch = ref->as <Branch> ();
		for (auto &segment : branch.segments)
			fn(segment.cond);
		break;
	}
	vcase(Construct): {
		auto &ctor = ref->as <Construct> ();
		for (auto &arg : ctor.args)
			fn(arg);
		break;
	}
	vcase(FieldAccess): {
		auto &facc = ref->as <FieldAccess> ();
		fn(facc.value);
		break;
	}
	vcase(Invocation): {
		auto &inv = ref->as <Invocation> ();
		for (auto &arg : inv.args)
			fn(arg);
		break;
	}
	vcase(Local): {
		auto &local = ref->as <Local> ();
		fn(local.init);
		break;
	}
	vcase(Operation): {
		auto &opn = ref->as <Operation> ();
		fn(opn.a);
		fn(opn.b);
		break;
	}
	vcase(Store): {
		auto &store = ref->as <Store> ();
		fn(store.source);
		break;
	}
	vcase(Swizzle): {
		auto &swz = ref->as <Swizzle> ();
		fn(swz.value);
		break;
	}
	default:
		break;
	}
}

bool is_readability_root(const Reference &ref)
{
	if (ref->is <Local> ())
		return true;
	if (ref->is <Store> ())
		return true;
	if (ref->is <Branch> ())
		return true;
	if (ref->is <Loop> ())
		return true;
	if (ref->is <Invocation> ())
		return true;
	if (ref->is <BuiltinIntrinsic> ()) {
		auto &bintr = ref->as <BuiltinIntrinsic> ();
		return has_side_effects(bintr.code);
	}

	return false;
}

void assign_readability_depth(
	const Reference &ref,
	size_t depth,
	std::map <Reference, size_t> &depths
)
{
	if (not ref)
		return;

	auto it = depths.find(ref);
	if (it != depths.end() and it->second >= depth)
		return;

	depths[ref] = depth;
	for_each_expression_operand(ref, [&](const Reference &opd) {
		assign_readability_depth(opd, depth + 1, depths);
	});
}

std::map <Reference, size_t> readability_depth_map(const SharedBlockReference &sbr)
{
	std::vector <SharedBlockReference> blocks;
	collect_blocks(sbr, blocks);

	std::map <Reference, size_t> depths;
	for (auto &block : blocks) {
		for (auto &instr : *block) {
			if (not is_readability_root(instr))
				continue;

			for_each_expression_operand(instr, [&](const Reference &opd) {
				assign_readability_depth(opd, 1, depths);
			});
		}
	}

	return depths;
}

std::map <Reference, size_t> instruction_order_map(const SharedBlockReference &sbr)
{
	std::vector <SharedBlockReference> blocks;
	collect_blocks(sbr, blocks);

	std::map <Reference, size_t> order;
	size_t index = 0;
	for (auto &block : blocks) {
		for (auto &instr : *block)
			order.emplace(instr, index++);
	}

	return order;
}

bool promote_to_local(const WholeBlockStructure &wbs, const Reference &promote)
{
	auto &origin = wbs.blocks.at(promote);

	auto type = get_or_add_type_ref(origin, promote);

	// Cannot promote array types to locals in GLSL
	auto &t = type->as <Type> ();
	if (t.is <Array> ())
		return false;

	auto index = std::find(origin->begin(), origin->end(), promote) - origin->begin();
	auto local = origin->add(index + 1, Local { type, promote });

	auto &uses = wbs.o2i.at(promote).ordered;
	for (auto &user : uses) {
		user->apply([&](Reference &opd) {
			if (opd == promote)
				opd = local;
		});
	}

	return true;
}

using RefMetric = std::pair <Reference, size_t>;

bool reuse_pass(const SharedBlockReference &sbr)
{
	bool changed = false;

	while (true) {
		auto wbs = build_whole_block_structure(sbr);

		std::vector <RefMetric> sea;
		for (auto &[opd, uses] : wbs.o2i)
			sea.emplace_back(opd, uses.ordered.size());

		std::ranges::sort(sea, std::ranges::greater(), &RefMetric::second);

		bool promoted = false;
		for (auto &[ref, count] : sea) {
			if (ref->is <Argument> ()
				or ref->is <Local> ()
				or ref->is <GlobalResource> ()
				or ref->is <Type> ())
				continue;

			if (count < 2)
				continue;

			// TODO: skip intrinsics with side effects

			if (promote_to_local(wbs, ref)) {
				promoted = true;
				break;
			}
		}

		if (not promoted)
			break;

		changed = true;
	}

	return changed;
}

void readability_pass(const SharedBlockReference &sbr)
{
	// Promote long expression trees into locals
	constexpr size_t long_expression_threshold = 5;
	while (true) {
		auto wbs = build_whole_block_structure(sbr);
		std::map <Reference, size_t> tree_sizes;
		auto depths = readability_depth_map(sbr);
		auto order = instruction_order_map(sbr);

		std::vector <Reference> sea;
		for (auto &[opd, uses] : wbs.o2i) {
			if (not is_promotable_expression(opd))
				continue;
			if (uses.ordered.empty())
				continue;
			if (has_only_local_init_uses(opd, uses.ordered))
				continue;

			auto size = expression_tree_size(opd, tree_sizes);
			if (size < long_expression_threshold)
				continue;

			sea.emplace_back(opd);
		}

		std::ranges::sort(sea, [&](const Reference &lhs, const Reference &rhs) {
			auto ldepth = depths.contains(lhs) ? depths.at(lhs) : 0;
			auto rdepth = depths.contains(rhs) ? depths.at(rhs) : 0;
			if (ldepth != rdepth)
				return ldepth > rdepth;

			auto lsize = tree_sizes.at(lhs);
			auto rsize = tree_sizes.at(rhs);
			if (lsize != rsize)
				return lsize > rsize;

			return order.at(lhs) < order.at(rhs);
		});
		bool promoted = false;
		for (auto &candidate : sea) {
			if (promote_to_local(wbs, candidate)) {
				promoted = true;
				break;
			}
		}
		if (not promoted)
			break;
	}
}

void optimize(const SharedBlockReference &sbr, OptimizationPhases phases)
{
	while (true) {
		bool changed = false;
		if (has_flag(phases, OptimizationPhases::eDeadCodeElimination))
			changed |= dead_code_elimination_pass(sbr);
		if (has_flag(phases, OptimizationPhases::eLocalElision))
			changed |= local_elision_pass(sbr);
		if (has_flag(phases, OptimizationPhases::eDeadCodeElimination))
			changed |= dead_code_elimination_pass(sbr);
		if (has_flag(phases, OptimizationPhases::eReuse))
			changed |= reuse_pass(sbr);

		if (not changed)
			break;
	}

	if (has_flag(phases, OptimizationPhases::eReadability))
		readability_pass(sbr);
}

} // namespace rcgp
