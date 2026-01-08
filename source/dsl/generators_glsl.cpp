#include "dsl/generators.hpp"
#include "meta/static_string.hpp"
#include "util/logging.hpp"

#include <fmt/format.h>

#include <type_traits>

namespace {

struct GLSLContext {
	const Block &block;

	std::vector <std::string> thread_inputs;
	std::vector <std::string> argument_names;
	std::map <uint32_t, std::string> local_thread_outputs;
	std::map <const Instruction *, std::string> local_names;
	uint32_t local_count = 0;

	std::map <const AggregateType *, std::string> aggregate_names;
	std::map <const Block *, std::string> subroutine_names;
	std::set <const Block *> emitted_subroutines;

	const Block *active_block = nullptr;

	std::string result;
	std::string indent = "    ";
};

bool is_same_type(const Reference &a, const Reference &b)
{
	if (a.get() == b.get())
		return true;
	if (!a || !b)
		return false;
	if (!a->is <Type> () || !b->is <Type> ())
		return false;

	auto &ta = a->as <Type> ();
	auto &tb = b->as <Type> ();
	if (ta.index() != tb.index())
		return false;

	vswitch (ta) {
	vcase(PrimitiveType):
		return ta.as <PrimitiveType> ().index() == tb.as <PrimitiveType> ().index();
	vcase(AggregateType): {
		auto &aa = ta.as <AggregateType> ();
		auto &bb = tb.as <AggregateType> ();
		if (aa.size() != bb.size())
			return false;
		for (size_t i = 0; i < aa.size(); i++) {
			if (!is_same_type(aa[i], bb[i]))
				return false;
		}
		return true;
	}
	vcase(ArrayType): {
		auto &aa = ta.as <ArrayType> ();
		auto &bb = tb.as <ArrayType> ();
		if (aa.size != bb.size)
			return false;
		return is_same_type(aa.base, bb.base);
	}
	default:
		break;
	}

	return false;
}

std::string rate_qualifier(RateProperties properties)
{
	switch (properties) {
	case RateProperties::eFlat: return "flat ";
	case RateProperties::eNone: return "";
	case RateProperties::eNoPerspective: return "noperspective ";
	case RateProperties::eSmooth: return "smooth ";
	default:
		break;
	}

	return "? ";
}

std::string layout_string(GlobalResourceLayout layout)
{
	switch (layout) {
	case GlobalResourceLayout::eScalar: return "scalar, ";
	case GlobalResourceLayout::eStd430: return "std430, ";
	case GlobalResourceLayout::eNone:
	default:
		return "";
	}
}

std::string type_name(GLSLContext &ctx, const PrimitiveType &primitive)
{
	vswitch (primitive) {
	vcase(int32_t): return "int";
	vcase(uint32_t): return "uint";
	vcase(float): return "float";
	vcase(bool): return "bool";
	vcase(VectorType <float, 2>): return "vec2";
	vcase(VectorType <float, 3>): return "vec3";
	vcase(VectorType <float, 4>): return "vec4";
	vcase(MatrixType <int32_t, 2, 2>): return "imat2";
	vcase(MatrixType <int32_t, 3, 3>): return "imat3";
	vcase(MatrixType <int32_t, 4, 4>): return "imat4";
	vcase(MatrixType <uint32_t, 2, 2>): return "umat2";
	vcase(MatrixType <uint32_t, 3, 3>): return "umat3";
	vcase(MatrixType <uint32_t, 4, 4>): return "umat4";
	vcase(MatrixType <float, 2, 2>): return "mat2";
	vcase(MatrixType <float, 3, 3>): return "mat3";
	vcase(MatrixType <float, 4, 4>): return "mat4";
	default:
		break;
	}

	return "?";
}

std::string type_name(GLSLContext &ctx, const Type &type)
{
	return std::visit([&](const auto &x) { return type_name(ctx, x); }, type);
}

std::string type_name(GLSLContext &ctx, const ArrayType &array)
{
	auto size = (array.size > 0)
		? fmt::format("[{}]", array.size)
		: "[]";

	if (!array.base || !array.base->is <Type> ())
		fatal("invalid array base type");

	return type_name(ctx, array.base->as <Type> ()) + size;
}

struct TypeDecl {
	std::string base;
	std::string suffix;
};

bool contains_unsized_array(Reference type)
{
	if (!type || !type->is <Type> ())
		return false;

	auto &t = type->as <Type> ();
	if (t.is <ArrayType> ()) {
		auto &arr = t.as <ArrayType> ();
		if (arr.size <= 0)
			return true;
		return contains_unsized_array(arr.base);
	}

	if (t.is <AggregateType> ()) {
		auto &agg = t.as <AggregateType> ();
		for (auto &field : agg) {
			if (contains_unsized_array(field))
				return true;
		}
	}

	return false;
}

bool contains_unsized_array(const AggregateType &agg)
{
	for (auto &field : agg) {
		if (contains_unsized_array(field))
			return true;
	}
	return false;
}

TypeDecl type_decl(GLSLContext &ctx, const Type &type)
{
	if (type.is <ArrayType> ()) {
		auto &array = type.as <ArrayType> ();
		if (!array.base || !array.base->is <Type> ())
			fatal("invalid array base type");
		auto decl = type_decl(ctx, array.base->as <Type> ());
		auto size = (array.size > 0)
			? fmt::format("[{}]", array.size)
			: "[]";
		decl.suffix += size;
		return decl;
	}

	return TypeDecl { type_name(ctx, type), "" };
}

TypeDecl type_decl(GLSLContext &ctx, Reference type)
{
	return std::visit([&](const auto &x) -> TypeDecl {
		using T = std::decay_t <decltype(x)>;
		if constexpr (std::is_same_v <T, Type>)
			return type_decl(ctx, x);
		if constexpr (std::is_same_v <T, PrimitiveType>)
			return TypeDecl { type_name(ctx, x), "" };
		if constexpr (std::is_same_v <T, ArrayType>) {
			auto tmp = Type { x };
			return type_decl(ctx, tmp);
		}
		if constexpr (std::is_same_v <T, AggregateType>)
			return TypeDecl { type_name(ctx, x), "" };
		auto type_name = std::string($ss_type(T).view());
		fatal("type decl not implemented for %s", type_name.c_str());
		return TypeDecl { "?", "" };
	}, *type);
}

std::string type_name(GLSLContext &ctx, const AggregateType &aggregate)
{
	auto addr = &aggregate;
	if (ctx.aggregate_names.contains(addr))
		return ctx.aggregate_names[addr];
	if (contains_unsized_array(aggregate))
		fatal("aggregate type with unsized array is not supported outside buffer blocks");
	return "?";
}

std::string type_name(GLSLContext &ctx, Reference type)
{
	return std::visit([&](const auto &x) -> std::string {
		using T = std::decay_t <decltype(x)>;
		if constexpr (std::is_same_v <T, Type>)
			return type_name(ctx, x);
		if constexpr (std::is_same_v <T, PrimitiveType>)
			return type_name(ctx, x);
		if constexpr (std::is_same_v <T, ArrayType>)
			return type_name(ctx, x);
		if constexpr (std::is_same_v <T, AggregateType>)
			return type_name(ctx, x);
		auto type_name = std::string($ss_type(T).view());
		fatal("type not implemented for %s", type_name.c_str());
		return "?";
	}, *type);
}

std::string reference_local(GLSLContext &ctx, Reference ref)
{
	auto ptr = ref.get();
	auto it = ctx.local_names.find(ptr);
	if (it != ctx.local_names.end())
		return it->second;

	auto name = fmt::format("lvar{}", ctx.local_count++);
	ctx.local_names.emplace(ptr, name);
	return name;
}

std::string resource_base_name(const GlobalResource &grsrc)
{
	auto group = grsrc.group.value_or(0);
	auto index = grsrc.index.value_or(0);
	if (grsrc.kind == GlobalResourceKind::eSampler)
		return fmt::format("s{}_i{}", group, index);
	return fmt::format("r{}_i{}", group, index);
}

std::string reference(GLSLContext &ctx, GlobalIntrinsic gi)
{
	switch (gi) {
	case GlobalIntrinsic::eScreenPosition: return "gl_Position";
	case GlobalIntrinsic::eInstanceIndex: return "gl_InstanceIndex";
	default:
		return "?";
	}
}

std::string reference(GLSLContext &ctx, GlobalResource grsrc)
{
	if (grsrc.kind == GlobalResourceKind::ePushConstant) {
		auto idx = grsrc.push_constant_index.value_or(0);
		return fmt::format("pc{}", idx);
	}

	auto base = resource_base_name(grsrc);

	if (!grsrc.type || !grsrc.type->is <Type> ())
		return base;

	auto &type = grsrc.type->as <Type> ();
	if (type.is <AggregateType> ())
		return base;

	return base + ".value";
}

std::string reference(GLSLContext &ctx, ThreadOutput tout)
{
	if (ctx.active_block
		&& ctx.active_block->context.model == ShaderStage::eSubroutine
		&& ctx.local_thread_outputs.contains(tout.argi)) {
		return ctx.local_thread_outputs[tout.argi];
	}

	return fmt::format("lout{}", tout.argi);
}

std::string reference(GLSLContext &ctx, Argument arg)
{
	if (arg.argi < ctx.argument_names.size())
		return ctx.argument_names[arg.argi];
	return fmt::format("arg{}", arg.argi);
}

std::string reference(GLSLContext &ctx, Reference ref)
{
	return std::visit([&](auto x) -> std::string {
		using T = std::decay_t <decltype(x)>;
		if constexpr (std::is_same_v <T, Local>)
			return reference_local(ctx, ref);
		if constexpr (std::is_same_v <T, GlobalIntrinsic>)
			return reference(ctx, x);
		if constexpr (std::is_same_v <T, GlobalResource>)
			return reference(ctx, x);
		if constexpr (std::is_same_v <T, ThreadOutput>)
			return reference(ctx, x);
		if constexpr (std::is_same_v <T, Argument>)
			return reference(ctx, x);
		auto type_name = std::string($ss_type(T).view());
		fatal("reference not implemented for %s", type_name.c_str());
		return "?";
	}, *ref);
}

std::string expression(GLSLContext &ctx, Reference expr)
{
	return std::visit([&](auto x) -> std::string {
		using T = std::decay_t <decltype(x)>;
		if constexpr (std::is_same_v <T, Local>)
			return reference(ctx, expr);
		if constexpr (std::is_same_v <T, Constant>) {
			if (x.template is <int32_t> ())
				return fmt::format("{}", x.template as <int32_t> ());
			if (x.template is <uint32_t> ())
				return fmt::format("{}", x.template as <uint32_t> ());
			if (x.template is <bool> ())
				return fmt::format("{}", x.template as <bool> ());
			if (x.template is <float> ())
				return fmt::format("{}", x.template as <float> ());
			return "?";
		}
		if constexpr (std::is_same_v <T, Operation>) {
			std::string op = "?";
			switch (x.code) {
			case OperationCode::eAdd: op = "+"; break;
			case OperationCode::eSubtract: op = "-"; break;
			case OperationCode::eMultiply: op = "*"; break;
			case OperationCode::eDivide: op = "/"; break;
			case OperationCode::eEqual: op = "=="; break;
			case OperationCode::eNotEqual: op = "!="; break;
			case OperationCode::eLess: op = "<"; break;
			case OperationCode::eLessEqual: op = "<="; break;
			case OperationCode::eGreater: op = ">"; break;
			case OperationCode::eGreaterEqual: op = ">="; break;
			default:
				break;
			}

			return fmt::format("({} {} {})",
				expression(ctx, x.a), op, expression(ctx, x.b));
		}
		if constexpr (std::is_same_v <T, ThreadInput>)
			return ctx.thread_inputs[x.argi];
		if constexpr (std::is_same_v <T, Construct>) {
			std::string out = type_name(ctx, x.type) + "(";

			auto nargs = x.args.size();
			for (size_t i = 0; i < nargs; i++) {
				out += expression(ctx, x.args[i]);
				if (i + 1 < nargs)
					out += ", ";
			}

			return out + ")";
		}
		if constexpr (std::is_same_v <T, FieldAccess>)
			return fmt::format("{}.f{}", expression(ctx, x.value), x.fidx);
		if constexpr (std::is_same_v <T, ArrayAccess>)
			return fmt::format("{}[{}]", expression(ctx, x.value), expression(ctx, x.index));
		if constexpr (std::is_same_v <T, Swizzle>)
			return fmt::format("{}.{}", expression(ctx, x.value), swizzle_string(x.code));
		if constexpr (std::is_same_v <T, GlobalResource>)
			return reference(ctx, x);
		if constexpr (std::is_same_v <T, GlobalIntrinsic>)
			return reference(ctx, x);
		if constexpr (std::is_same_v <T, Argument>)
			return reference(ctx, x);
		if constexpr (std::is_same_v <T, Invocation>) {
			const Block *callee = x.sbr.get();
			auto it = ctx.subroutine_names.find(callee);
			std::string name = (it != ctx.subroutine_names.end())
				? it->second
				: fmt::format("fn_{}", (void *) callee);

			std::string out = name + "(";
			auto nargs = x.args.size();
			for (size_t i = 0; i < nargs; i++) {
				out += expression(ctx, x.args[i]);
				if (i + 1 < nargs)
					out += ", ";
			}

			return out + ")";
		}
		if constexpr (std::is_same_v <T, BuiltinIntrinsic>) {
			std::string out = "?";

			switch (x.code) {
			case BuiltinIntrinsicCode::eCross: out = "cross"; break;
			case BuiltinIntrinsicCode::eDFdx: out = "dFdx"; break;
			case BuiltinIntrinsicCode::eDFdxFine: out = "dFdxFine"; break;
			case BuiltinIntrinsicCode::eDFdy: out = "dFdy"; break;
			case BuiltinIntrinsicCode::eDFdyFine: out = "dFdyFine"; break;
			case BuiltinIntrinsicCode::eDot: out = "dot"; break;
			case BuiltinIntrinsicCode::eInverse: out = "inverse"; break;
			case BuiltinIntrinsicCode::eMax: out = "max"; break;
			case BuiltinIntrinsicCode::eNormalize: out = "normalize"; break;
			case BuiltinIntrinsicCode::eSample: out = "texture"; break;
			case BuiltinIntrinsicCode::eTranspose: out = "transpose"; break;
			default:
				break;
			}

			out += "(";

			auto nargs = x.args.size();
			for (size_t i = 0; i < nargs; i++) {
				out += expression(ctx, x.args[i]);
				if (i + 1 < nargs)
					out += ", ";
			}

			return out + ")";
		}

		auto type_name = std::string($ss_type(T).view());
		fatal("expression not implemented for %s", type_name.c_str());
		return "?";
	}, *expr);
}

void statement(GLSLContext &ctx, Reference instruction)
{
	std::visit([&](auto x) {
		using T = std::decay_t <decltype(x)>;
		if constexpr (std::is_same_v <T, Local>) {
			auto name = reference(ctx, instruction);
			auto decl = type_decl(ctx, x.type);
			ctx.result += fmt::format("{}{} {}{};\n",
				ctx.indent, decl.base, name, decl.suffix);
			return;
		}
		if constexpr (std::is_same_v <T, Store>) {
			ctx.result += fmt::format("{}{} = {};\n",
				ctx.indent,
				reference(ctx, x.destination),
				expression(ctx, x.source));
			return;
		}
		if constexpr (std::is_same_v <T, Invocation>) {
			ctx.result += fmt::format("{}{};\n",
				ctx.indent,
				expression(ctx, instruction));
			return;
		}
		if constexpr (std::is_same_v <T, Branch>) {
			auto emit_body = [&](const SharedBlockReference &blk) {
				auto prev = ctx.indent;
				ctx.indent += "    ";
				for (auto &nested : *blk) {
					if (nested->is <Store> () || nested->is <Invocation> ()
						|| nested->is <Branch> () || nested->is <Loop> ()
						|| nested->is <Local> ())
						statement(ctx, nested);
				}
				ctx.indent = prev;
			};

			for (size_t i = 0; i < x.segments.size(); i++) {
				auto &segment = x.segments[i];
				auto kw = (i == 0) ? "if" : "else if";
				ctx.result += fmt::format("{}{} ({})\n",
					ctx.indent, kw, expression(ctx, segment.cond));
				ctx.result += fmt::format("{}{{\n", ctx.indent);
				emit_body(segment.body);
				ctx.result += fmt::format("{}}}\n", ctx.indent);
			}

			if (x.fallback.has_value()) {
				ctx.result += fmt::format("{}else\n", ctx.indent);
				ctx.result += fmt::format("{}{{\n", ctx.indent);
				emit_body(x.fallback.value());
				ctx.result += fmt::format("{}}}\n", ctx.indent);
			}
			return;
		}
		if constexpr (std::is_same_v <T, Loop>) {
			auto emit_body = [&](const SharedBlockReference &blk) {
				auto prev = ctx.indent;
				ctx.indent += "    ";
				for (auto &nested : *blk) {
					if (nested->is <Store> () || nested->is <Invocation> ()
						|| nested->is <Branch> () || nested->is <Loop> ()
						|| nested->is <Local> ())
						statement(ctx, nested);
				}
				ctx.indent = prev;
			};

			auto cond_expr = expression(ctx, x.cond_value);

			if (x.kind == LoopKind::eFor && x.init.has_value())
				emit_body(x.init.value());

			auto loop_kw = (x.kind == LoopKind::eFor) ? "for (;;)" : "while (true)";
			ctx.result += fmt::format("{}{}\n", ctx.indent, loop_kw);
			ctx.result += fmt::format("{}{{\n", ctx.indent);
			emit_body(x.cond);
			ctx.result += fmt::format("{}    if (!({}))\n", ctx.indent, cond_expr);
			ctx.result += fmt::format("{}        break;\n", ctx.indent);
			emit_body(x.body);
			if (x.kind == LoopKind::eFor && x.step.has_value())
				emit_body(x.step.value());
			ctx.result += fmt::format("{}}}\n", ctx.indent);
			return;
		}

		ctx.result += fmt::format("{}?\n", ctx.indent);
	}, *instruction);
}

void emit_block_statements(GLSLContext &ctx, const Block &blk)
{
	for (auto &instr : blk) {
		if (instr->is <Store> () || instr->is <Invocation> ()
			|| instr->is <Branch> () || instr->is <Loop> ()
			|| instr->is <Local> ())
			statement(ctx, instr);
	}
}

void set_active_block(GLSLContext &ctx, const Block &blk)
{
	ctx.active_block = &blk;
}

void reset_state(GLSLContext &ctx)
{
	ctx.result.clear();
	ctx.aggregate_names.clear();
	ctx.subroutine_names.clear();
	ctx.emitted_subroutines.clear();
	ctx.thread_inputs.clear();
	ctx.argument_names.clear();
	ctx.local_thread_outputs.clear();
	ctx.local_names.clear();
	ctx.local_count = 0;
	set_active_block(ctx, ctx.block);
}

void collect_blocks(const GLSLContext &ctx, std::vector <const Block *> &blocks)
{
	std::set <const Block *> visited;

	auto visit = [&](auto &&self, const Block &blk) -> void {
		if (visited.contains(&blk))
			return;
		visited.emplace(&blk);
		blocks.push_back(&blk);

		for (auto &instr : blk) {
			if (instr->is <Invocation> ()) {
				auto &inv = instr->as <Invocation> ();
				self(self, *inv.sbr);
			} else if (instr->is <Branch> ()) {
				auto &branch = instr->as <Branch> ();
				for (auto &segment : branch.segments)
					self(self, *segment.body);
				if (branch.fallback.has_value())
					self(self, *branch.fallback.value());
			} else if (instr->is <Loop> ()) {
				auto &loop = instr->as <Loop> ();
				if (loop.init.has_value())
					self(self, *loop.init.value());
				self(self, *loop.cond);
				if (loop.step.has_value())
					self(self, *loop.step.value());
				self(self, *loop.body);
			}
		}
	};

	visit(visit, ctx.block);
}

void emit_preamble(GLSLContext &ctx)
{
	ctx.result = "// Preamble\n";
	ctx.result += "#version 460\n\n";
	ctx.result += "#extension GL_EXT_scalar_block_layout : require\n\n";
}

void emit_aggregate_decls(GLSLContext &ctx)
{
	ctx.result += "// Types\n";

	std::vector <const Block *> blocks;
	collect_blocks(ctx, blocks);

	size_t aggregate_counter = 0;
	for (auto *blk : blocks) {
		for (auto &instr : *blk) {
			if (not instr->is <Type> ())
				continue;

			auto &type = instr->as <Type> ();
			if (not type.is <AggregateType> ())
				continue;

			auto &agg = type.as <AggregateType> ();
			if (contains_unsized_array(agg))
				continue;

			ctx.result += fmt::format("struct AGG{} {{\n", aggregate_counter++);

			size_t field_counter = 0;
			for (auto &field : agg) {
				auto decl = type_decl(ctx, field);
				ctx.result += fmt::format("    {} f{}{};\n",
					decl.base, field_counter++, decl.suffix);
			}

			ctx.result += "};\n\n";

			ctx.aggregate_names.emplace(&agg, fmt::format("AGG{}", aggregate_counter - 1));
		}
	}

	ctx.result += "\n";
}

void emit_thread_inputs(GLSLContext &ctx)
{
	ctx.result += "// Inputs\n";

	ctx.thread_inputs.reserve(ctx.block.context.thread_inputs.size());
	for (auto &tin : ctx.block.context.thread_inputs) {
		ctx.thread_inputs.push_back(fmt::format("lin{}", tin.argi));
		auto decl = type_decl(ctx, tin.type);
		ctx.result += fmt::format("layout (location = {}) in {} lin{}{};\n",
			tin.argi, decl.base, tin.argi, decl.suffix);
	}

	if (ctx.block.context.thread_inputs.size())
		ctx.result += "\n";
	else
		ctx.result += "\n";
}

void emit_thread_outputs(GLSLContext &ctx)
{
	ctx.result += "// Outputs\n";

	std::map <uint32_t, ThreadOutput> outputs;
	std::vector <const Block *> blocks;
	collect_blocks(ctx, blocks);

	for (auto *blk : blocks) {
		for (auto &tout : blk->context.thread_outputs) {
			auto it = outputs.find(tout.argi);
			if (it == outputs.end()) {
				outputs.emplace(tout.argi, tout);
				continue;
			}

			if (!is_same_type(it->second.type, tout.type)
				|| it->second.properties != tout.properties)
				fatal("thread output mismatch for location %u", tout.argi);
		}
	}

	for (auto &[_, tout] : outputs) {
		auto qualifier = rate_qualifier(tout.properties);
		auto decl = type_decl(ctx, tout.type);
		ctx.result += fmt::format("layout (location = {}) {}out {} lout{}{};\n",
			tout.argi, qualifier, decl.base, tout.argi, decl.suffix);
	}

	if (outputs.size())
		ctx.result += "\n";
	else
		ctx.result += "\n";
}

void collect_push_constant_indices(const std::vector <const Block *> &blocks,
	std::map <void *, uint32_t> &pc_indices)
{
	uint32_t pcounter = 0;
	for (auto *blk : blocks) {
		for (auto &[addr, refs] : blk->context.global_resources) {
			for (auto &ref : refs) {
				auto &grsrc = ref->as <GlobalResource> ();
				if (grsrc.kind != GlobalResourceKind::ePushConstant)
					continue;

				auto it = pc_indices.find(addr);
				if (it == pc_indices.end())
					it = pc_indices.emplace(addr, pcounter++).first;
				grsrc.push_constant_index = it->second;
			}
		}
	}
}

std::string resource_key(const GlobalResource &grsrc)
{
	if (grsrc.kind == GlobalResourceKind::eSampler) {
		auto group = grsrc.group.value_or(0);
		auto index = grsrc.index.value_or(0);
		return fmt::format("sampler:{}:{}", group, index);
	}

	if (grsrc.kind == GlobalResourceKind::ePushConstant) {
		auto idx = grsrc.push_constant_index.value_or(0);
		auto offset = grsrc.push_constant_offset.value_or(0);
		return fmt::format("pc:{}:{}:{}:{}",
			idx, offset, (int) grsrc.layout, (void *) grsrc.type.get());
	}

	auto group = grsrc.group.value_or(0);
	auto index = grsrc.index.value_or(0);
	return fmt::format("buf:{}:{}:{}:{}",
		(int) grsrc.kind, group, index, (int) grsrc.layout);
}

std::string resource_instance_name(const GlobalResource &grsrc)
{
	return resource_base_name(grsrc);
}

void emit_buffer_fields(GLSLContext &ctx, const AggregateType &agg)
{
	for (size_t i = 0; i < agg.size(); i++) {
		if (contains_unsized_array(agg[i]) && (i + 1 < agg.size()))
			fatal("unsized array must be the last field in a buffer block");
		auto decl = type_decl(ctx, agg[i]);
		ctx.result += fmt::format("    {} f{}{};\n", decl.base, i, decl.suffix);
	}
}

void emit_resource_decl(GLSLContext &ctx, GlobalResource &grsrc)
{
	if (grsrc.kind == GlobalResourceKind::eSampler) {
		auto group = grsrc.group.value_or(0);
		auto index = grsrc.index.value_or(0);
		ctx.result += fmt::format("layout (set = {}, binding = {}) uniform sampler2D {};\n\n",
			group, index, resource_base_name(grsrc));
		return;
	}

	if (grsrc.kind == GlobalResourceKind::ePushConstant) {
		auto idx = grsrc.push_constant_index.value_or(0);
		grsrc.push_constant_index = idx;
		auto offset = grsrc.push_constant_offset.value_or(0);
		auto decl = type_decl(ctx, grsrc.type);

		ctx.result += fmt::format("layout ({}push_constant) uniform PC{} {{\n",
			layout_string(grsrc.layout), idx);
		ctx.result += fmt::format("    layout (offset = {}) {} pc{}{};\n",
			offset, decl.base, idx, decl.suffix);
		ctx.result += "};\n\n";
		return;
	}

	std::string modifier;
	switch (grsrc.kind) {
	case GlobalResourceKind::eUniformBuffer: modifier = "uniform"; break;
	case GlobalResourceKind::eStorageBuffer: modifier = "buffer"; break;
	default:
		fatal("unsupported global resource kind");
	}

	auto group = grsrc.group.value_or(0);
	auto index = grsrc.index.value_or(0);
	auto instance = resource_instance_name(grsrc);
	ctx.result += fmt::format("layout ({}set = {}, binding = {}) {} R{}{} {{\n",
		layout_string(grsrc.layout), group, index, modifier, group, index);
	if (!grsrc.type || !grsrc.type->is <Type> ())
		fatal("invalid resource type");

	auto &type = grsrc.type->as <Type> ();
	if (type.is <AggregateType> ()) {
		emit_buffer_fields(ctx, type.as <AggregateType> ());
	} else {
		auto decl = type_decl(ctx, grsrc.type);
		ctx.result += fmt::format("    {} value{};\n", decl.base, decl.suffix);
	}
	ctx.result += fmt::format("}} {};\n\n", instance);
}

void emit_global_resources(GLSLContext &ctx)
{
	std::vector <const Block *> blocks;
	collect_blocks(ctx, blocks);

	std::map <void *, uint32_t> pc_indices;
	collect_push_constant_indices(blocks, pc_indices);

	ctx.result += "// Resources\n";

	std::set <std::string> emitted;
	for (auto *blk : blocks) {
		for (auto &[_, refs] : blk->context.global_resources) {
			for (auto &ref : refs) {
				auto &grsrc = ref->as <GlobalResource> ();
				auto key = resource_key(grsrc);
				if (emitted.contains(key))
					continue;
				emitted.emplace(key);
				emit_resource_decl(ctx, grsrc);
			}
		}
	}

	ctx.result += "\n";
}

std::string subroutine_return_type(GLSLContext &ctx, const Block &blk, uint32_t &out_argi)
{
	out_argi = 0;
	if (blk.context.thread_outputs.empty())
		return "void";
	if (blk.context.thread_outputs.size() > 1)
		fatal("subroutine return with multiple outputs is not supported");

	auto &tout = blk.context.thread_outputs.front();
	out_argi = tout.argi;
	return type_name(ctx, tout.type);
}

void emit_subroutine_function(GLSLContext &ctx, const Block &blk, const std::string &name)
{
	set_active_block(ctx, blk);
	ctx.argument_names.clear();
	ctx.local_thread_outputs.clear();

	for (auto &arg : blk.context.arguments)
		ctx.argument_names.push_back(fmt::format("arg{}", arg.argi));

	uint32_t ret_argi = 0;
	auto return_type = subroutine_return_type(ctx, blk, ret_argi);

	ctx.result += fmt::format("{} {}(", return_type, name);
	for (size_t i = 0; i < blk.context.arguments.size(); i++) {
		auto &arg = blk.context.arguments[i];
		auto decl = type_decl(ctx, arg.type);
		ctx.result += fmt::format("{} {}{}", decl.base, ctx.argument_names[arg.argi], decl.suffix);
		if (i + 1 < blk.context.arguments.size())
			ctx.result += ", ";
	}
	ctx.result += ")\n{\n";

	// TODO: should just be one set of returns...
	// for perf maybe even prefer out parameters for tuples
	for (auto &tout : blk.context.thread_outputs) {
		auto name = fmt::format("lout{}", tout.argi);
		ctx.local_thread_outputs.emplace(tout.argi, name);
		auto decl = type_decl(ctx, tout.type);
		ctx.result += fmt::format("    {} {}{};\n", decl.base, name, decl.suffix);
	}

	if (blk.context.thread_outputs.size())
		ctx.result += "\n";

	emit_block_statements(ctx, blk);

	if (return_type != "void") {
		auto name_it = ctx.local_thread_outputs.find(ret_argi);
		auto local_name = (name_it != ctx.local_thread_outputs.end())
			? name_it->second
			: fmt::format("lout{}", ret_argi);
		ctx.result += fmt::format("    return {};\n", local_name);
	}

	ctx.result += "}\n\n";
	set_active_block(ctx, ctx.block);
}

void emit_subroutine_functions(GLSLContext &ctx)
{
	ctx.result += "// Subroutines\n";

	std::vector <const Block *> order;
	std::set <const Block *> visited;

	auto visit = [&](auto &&self, const Block &blk) -> void {
		for (auto &instr : blk) {
			if (instr->is <Invocation> ()) {
				auto &inv = instr->as <Invocation> ();
				const Block *callee = inv.sbr.get();
				if (visited.contains(callee))
					continue;
				visited.emplace(callee);
				self(self, *callee);
				order.push_back(callee);
			}
		}
	};

	visit(visit, ctx.block);

	for (auto *callee : order) {
		if (callee->context.model != ShaderStage::eSubroutine)
			continue;

		auto it = ctx.subroutine_names.find(callee);
		if (it == ctx.subroutine_names.end()) {
			auto name = fmt::format("fn_{}", (void *) callee);
			it = ctx.subroutine_names.emplace(callee, name).first;
		}

		if (ctx.emitted_subroutines.contains(callee))
			continue;

		ctx.emitted_subroutines.emplace(callee);
		emit_subroutine_function(ctx, *callee, it->second);
	}
}

void emit_main_function(GLSLContext &ctx)
{
	ctx.result += "// Main\n";
	ctx.result += "void main()\n";
	ctx.result += "{\n";

	emit_block_statements(ctx, ctx.block);

	ctx.result += "}";
}

std::string generate(GLSLContext &ctx, size_t tabs)
{
	reset_state(ctx);

	if (ctx.block.context.model == ShaderStage::eSubroutine) {
		emit_aggregate_decls(ctx);
		auto name = fmt::format("fn_{}", (void *) &ctx.block);
		ctx.subroutine_names.emplace(&ctx.block, name);
		emit_subroutine_function(ctx, ctx.block, name);
		return ctx.result;
	}

	emit_preamble(ctx);
	emit_aggregate_decls(ctx);
	emit_thread_inputs(ctx);
	emit_thread_outputs(ctx);
	emit_global_resources(ctx);
	emit_subroutine_functions(ctx);
	emit_main_function(ctx);

	return ctx.result;
}

} // namespace

std::string generate_glsl(const SharedBlockReference &sbr, size_t tabs)
{
	GLSLContext ctx { *sbr.get() };
	return generate(ctx, tabs);
}
