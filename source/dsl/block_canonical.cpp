#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "dsl/block_canonical.hpp"
#include "dsl/block.hpp"
#include "dsl/instructions.hpp"
#include "util/error.hpp"

namespace rcgp {

static constexpr uint64_t FNV_PRIME = 1099511628211ull;
static constexpr uint64_t FNV_OFFSET_A = 14695981039346656037ull;
static constexpr uint64_t FNV_OFFSET_B = 0x84222325cbf29ce4ull;

auto fnv1a_128(std::span <const std::byte> data) -> std::array <uint64_t, 2>
{
	uint64_t a = FNV_OFFSET_A;
	uint64_t b = FNV_OFFSET_B;
	for (std::byte byte : data) {
		auto v = uint64_t(uint8_t(byte));
		a ^= v;
		a *= FNV_PRIME;
		b ^= v;
		b *= FNV_PRIME;
	}
	return { a, b };
}

auto fnv1a_128(std::string_view s) -> std::array <uint64_t, 2>
{
	auto ptr = reinterpret_cast <const std::byte *> (s.data());
	return fnv1a_128(std::span <const std::byte> (ptr, s.size()));
}

enum class Tag : uint8_t {
	eNone = 0x00,
	ePresent = 0x01,

	eBlockBegin = 0x10,
	eBlockEnd = 0x11,
	eBlockRefNew = 0x12,
	eBlockRefSeen = 0x13,

	eRefNew = 0x20,
	eRefSeen = 0x21,
	eRefNull = 0x22,

	eNodeType = 0x40,
	eNodeConstant = 0x41,
	eNodeOperation = 0x42,
	eNodeArgument = 0x43,
	eNodeReturn = 0x44,
	eNodeConstruct = 0x45,
	eNodeLocal = 0x46,
	eNodeStore = 0x47,
	eNodeLoop = 0x48,
	eNodeBranch = 0x49,
	eNodeSwizzle = 0x4a,
	eNodeFieldAccess = 0x4b,
	eNodeArrayAccess = 0x4c,
	eNodeSystemValue = 0x4d,
	eNodeStageInput = 0x4e,
	eNodeStageOutput = 0x4f,
	eNodeGlobalResource = 0x50,
	eNodeBuiltinIntrinsic = 0x51,
	eNodeInvocation = 0x52,

	eTypePrimitive = 0x60,
	eTypeStruct = 0x61,
	eTypeArray = 0x62,
	eTypeBufferRef = 0x63,

	eConstantBool = 0x70,
	eConstantI32 = 0x71,
	eConstantU32 = 0x72,
	eConstantF32 = 0x73,
	eConstantString = 0x74,
};

struct BlockCanonicalizer {
	std::vector <std::byte> bytes;
	std::unordered_map <const Instruction *, uint32_t> instruction_ids;
	std::unordered_map <const Block *, uint32_t> block_ids;
	std::unordered_set <const Block *> walked_blocks;

	void put(Tag tag) {
		bytes.push_back(std::byte(tag));
	}

	void put_u32(uint32_t v) {
		bytes.reserve(bytes.size() + 4);
		for (int i = 0; i < 4; i++)
			bytes.push_back(std::byte((v >> (i * 8)) & 0xff));
	}

	void put_u64(uint64_t v) {
		bytes.reserve(bytes.size() + 8);
		for (int i = 0; i < 8; i++)
			bytes.push_back(std::byte((v >> (i * 8)) & 0xff));
	}

	void put_i32(int32_t v) {
		put_u32(uint32_t(v));
	}

	void put_i64(int64_t v) {
		put_u64(uint64_t(v));
	}

	void put_f32(float v) {
		uint32_t u;
		std::memcpy(&u, &v, sizeof(u));
		put_u32(u);
	}

	void put_str(std::string_view s) {
		put_u32(uint32_t(s.size()));
		bytes.reserve(bytes.size() + s.size());
		for (char c : s)
			bytes.push_back(std::byte(uint8_t(c)));
	}

	void put_bool(bool v) {
		bytes.push_back(std::byte(v ? 1 : 0));
	}

	void emit_opt_u32(const std::optional <uint32_t> &v) {
		if (v) {
			put(Tag::ePresent);
			put_u32(*v);
		} else {
			put(Tag::eNone);
		}
	}

	void emit_opt_i64(const std::optional <int64_t> &v) {
		if (v) {
			put(Tag::ePresent);
			put_i64(*v);
		} else {
			put(Tag::eNone);
		}
	}

	void emit_ref(const Reference &ref) {
		if (!ref) {
			put(Tag::eRefNull);
			return;
		}

		auto *ptr = ref.get();
		auto it = instruction_ids.find(ptr);
		if (it != instruction_ids.end()) {
			put(Tag::eRefSeen);
			put_u32(it->second);
			return;
		}

		auto id = uint32_t(instruction_ids.size());
		instruction_ids.emplace(ptr, id);
		put(Tag::eRefNew);
		put_u32(id);
		emit_instruction_body(*ref);
	}

	void emit_sub_block(const SharedBlockReference &sbr) {
		if (!sbr) {
			put(Tag::eRefNull);
			return;
		}

		auto *ptr = sbr.get();
		auto it = block_ids.find(ptr);
		if (it != block_ids.end()) {
			put(Tag::eBlockRefSeen);
			put_u32(it->second);
			return;
		}

		auto id = uint32_t(block_ids.size());
		block_ids.emplace(ptr, id);
		put(Tag::eBlockRefNew);
		put_u32(id);
		walk_block_body(sbr);
	}

	void emit_type(const Type &t) {
		vswitch (t) {
		vcase(Primitive): {
			put(Tag::eTypePrimitive);
			put_u32(uint32_t(t.as <Primitive> ()));
			return;
		}
		vcase(Struct): {
			put(Tag::eTypeStruct);
			auto &st = t.as <Struct> ();
			put_str(st.name);
			put_u32(uint32_t(st.fields.size()));
			for (auto &name : st.fields)
				put_str(name);
			put_u32(uint32_t(st.size()));
			for (auto &field : st)
				emit_ref(field);
			return;
		}
		vcase(Array): {
			put(Tag::eTypeArray);
			auto &array = t.as <Array> ();
			put_i64(array.size);
			emit_ref(array.base);
			return;
		}
		vcase(BufferReferenceType): {
			put(Tag::eTypeBufferRef);
			auto &brt = t.as <BufferReferenceType> ();
			emit_ref(brt.element_type);
			put_u32(uint32_t(brt.layout));
			put_u32(uint32_t(brt.access));
			return;
		}
		default:
			fatal("block_canonical: unhandled Type variant");
		}
	}

	void emit_constant(const Constant &c) {
		std::visit([&] <typename T> (const T &x) {
			if constexpr (std::same_as <T, bool>) {
				put(Tag::eConstantBool);
				put_bool(x);
			} else if constexpr (std::same_as <T, int32_t>) {
				put(Tag::eConstantI32);
				put_i32(x);
			} else if constexpr (std::same_as <T, uint32_t>) {
				put(Tag::eConstantU32);
				put_u32(x);
			} else if constexpr (std::same_as <T, float>) {
				put(Tag::eConstantF32);
				put_f32(x);
			} else if constexpr (std::same_as <T, std::string>) {
				put(Tag::eConstantString);
				put_str(x);
			} else {
				static_assert(sizeof(T) == 0, "block_canonical: unhandled Constant variant");
			}
		}, c);
	}

	void emit_instruction_body(const Instruction &instr) {
		vswitch (instr) {
		vcase(Type): {
			put(Tag::eNodeType);
			emit_type(instr.as <Type> ());
			return;
		}
		vcase(Constant): {
			put(Tag::eNodeConstant);
			emit_constant(instr.as <Constant> ());
			return;
		}
		vcase(Operation): {
			put(Tag::eNodeOperation);
			auto &opn = instr.as <Operation> ();
			put_u32(uint32_t(opn.code));
			emit_ref(opn.a);
			emit_ref(opn.b);
			return;
		}
		vcase(Argument): {
			put(Tag::eNodeArgument);
			auto &arg = instr.as <Argument> ();
			put_u32(arg.argi);
			emit_ref(arg.type);
			return;
		}
		vcase(Return): {
			put(Tag::eNodeReturn);
			auto &ret = instr.as <Return> ();
			put_u32(ret.argi);
			emit_ref(ret.type);
			return;
		}
		vcase(Construct): {
			put(Tag::eNodeConstruct);
			auto &ctor = instr.as <Construct> ();
			emit_ref(ctor.type);
			put_u32(uint32_t(ctor.args.size()));
			for (auto &a : ctor.args)
				emit_ref(a);
			return;
		}
		vcase(Local): {
			put(Tag::eNodeLocal);
			auto &local = instr.as <Local> ();
			emit_ref(local.type);
			emit_ref(local.init);
			return;
		}
		vcase(Store): {
			put(Tag::eNodeStore);
			auto &store = instr.as <Store> ();
			emit_ref(store.destination);
			emit_ref(store.source);
			return;
		}
		vcase(Loop): {
			put(Tag::eNodeLoop);
			auto &loop = instr.as <Loop> ();
			emit_sub_block(loop.body);
			return;
		}
		vcase(Branch): {
			put(Tag::eNodeBranch);
			auto &branch = instr.as <Branch> ();
			put_u32(uint32_t(branch.segments.size()));
			for (auto &seg : branch.segments) {
				emit_ref(seg.cond);
				emit_sub_block(seg.body);
			}
			if (branch.fallback) {
				put(Tag::ePresent);
				emit_sub_block(*branch.fallback);
			} else {
				put(Tag::eNone);
			}
			return;
		}
		vcase(Swizzle): {
			put(Tag::eNodeSwizzle);
			auto &swz = instr.as <Swizzle> ();
			put_u32(uint32_t(swz.code));
			emit_ref(swz.value);
			return;
		}
		vcase(FieldAccess): {
			put(Tag::eNodeFieldAccess);
			auto &fa = instr.as <FieldAccess> ();
			put_u32(fa.fidx);
			emit_ref(fa.value);
			return;
		}
		vcase(ArrayAccess): {
			put(Tag::eNodeArrayAccess);
			auto &aa = instr.as <ArrayAccess> ();
			emit_ref(aa.value);
			emit_ref(aa.index);
			return;
		}
		vcase(SystemValue): {
			put(Tag::eNodeSystemValue);
			put_u32(uint32_t(instr.as <SystemValue> ()));
			return;
		}
		vcase(StageInput): {
			put(Tag::eNodeStageInput);
			auto &sin = instr.as <StageInput> ();
			put_u32(sin.argi);
			put_u32(uint32_t(sin.properties));
			emit_ref(sin.type);
			return;
		}
		vcase(StageOutput): {
			put(Tag::eNodeStageOutput);
			auto &sout = instr.as <StageOutput> ();
			put_u32(sout.argi);
			put_u32(uint32_t(sout.properties));
			emit_ref(sout.type);
			return;
		}
		vcase(GlobalResource): {
			put(Tag::eNodeGlobalResource);
			auto &gr = instr.as <GlobalResource> ();
			put_u32(uint32_t(gr.kind));
			put_u32(uint32_t(gr.layout));
			put_u32(uint32_t(gr.access));
			emit_opt_u32(gr.group);
			emit_opt_u32(gr.index);
			emit_opt_u32(gr.offset);
			emit_opt_i64(gr.count);
			emit_ref(gr.type);
			return;
		}
		vcase(BuiltinIntrinsic): {
			put(Tag::eNodeBuiltinIntrinsic);
			auto &bi = instr.as <BuiltinIntrinsic> ();
			put_u32(uint32_t(bi.code));
			put_u32(uint32_t(bi.args.size()));
			for (auto &a : bi.args)
				emit_ref(a);
			return;
		}
		vcase(Invocation): {
			put(Tag::eNodeInvocation);
			auto &inv = instr.as <Invocation> ();
			emit_sub_block(inv.sbr);
			put_u32(uint32_t(inv.args.size()));
			for (auto &a : inv.args)
				emit_ref(a);
			put_u32(uint32_t(inv.returns.size()));
			for (auto &r : inv.returns)
				emit_ref(r);
			return;
		}
		default:
			fatal("block_canonical: unhandled Instruction variant");
		}
	}

	void walk_block_body(const SharedBlockReference &sbr) {
		auto *ptr = sbr.get();
		if (walked_blocks.contains(ptr))
			return;
		walked_blocks.emplace(ptr);

		put(Tag::eBlockBegin);
		put_u32(uint32_t(sbr->stage));
		put_str(sbr->name);

		if (sbr->workgroup_size) {
			put(Tag::ePresent);
			put_u32((*sbr->workgroup_size)[0]);
			put_u32((*sbr->workgroup_size)[1]);
			put_u32((*sbr->workgroup_size)[2]);
		} else {
			put(Tag::eNone);
		}

		if (sbr->mesh_primitive_kind) {
			put(Tag::ePresent);
			put_u32(uint32_t(*sbr->mesh_primitive_kind));
		} else {
			put(Tag::eNone);
		}

		emit_opt_u32(sbr->mesh_max_primitives);
		emit_opt_u32(sbr->mesh_max_vertices);

		if (sbr->hit_attribute_type) {
			put(Tag::ePresent);
			emit_ref(*sbr->hit_attribute_type);
		} else {
			put(Tag::eNone);
		}

		if (sbr->task_payload_type) {
			put(Tag::ePresent);
			emit_ref(*sbr->task_payload_type);
		} else {
			put(Tag::eNone);
		}

		put_u32(uint32_t(sbr->mesh_perprimitive_outputs.size()));
		for (auto &[idx, flag] : sbr->mesh_perprimitive_outputs) {
			put_u32(idx);
			put_bool(flag);
		}

		put_u32(uint32_t(sbr->arguments.size()));
		for (auto &a : sbr->arguments) {
			put_u32(a.argi);
			emit_ref(a.type);
		}

		put_u32(uint32_t(sbr->returns.size()));
		for (auto &r : sbr->returns) {
			put_u32(r.argi);
			emit_ref(r.type);
		}

		put_u32(uint32_t(sbr->stage_inputs.size()));
		for (auto &sin : sbr->stage_inputs) {
			put_u32(sin.argi);
			put_u32(uint32_t(sin.properties));
			emit_ref(sin.type);
		}

		put_u32(uint32_t(sbr->stage_outputs.size()));
		for (auto &sout : sbr->stage_outputs) {
			put_u32(sout.argi);
			put_u32(uint32_t(sout.properties));
			emit_ref(sout.type);
		}

		put_u32(uint32_t(sbr->size()));
		for (auto &instr : *sbr)
			emit_ref(instr);

		put(Tag::eBlockEnd);
	}
};

auto canonicalize_block(const SharedBlockReference &sbr) -> BlockCanonical
{
	BlockCanonicalizer c;
	c.emit_sub_block(sbr);

	auto span = std::span <const std::byte> (c.bytes.data(), c.bytes.size());
	return BlockCanonical {
		.hash = fnv1a_128(span),
		.bytes = std::move(c.bytes),
	};
}

} // namespace rcgp
