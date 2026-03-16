#include "dsl/jems.hpp"
#include "dsl/block.hpp"
#include "dsl/instructions.hpp"
#include "dsl/tracer.hpp"

namespace rcgp::jems {

scope::scope(const SharedBlockReference &sbr)
{
	Tracer::singleton.records.emplace_back(sbr);
}

scope::~scope()
{
	Tracer::singleton.records.pop_back();
}

scope::operator bool() const
{
	return true;
}

void null::override_reference(const Reference &) {}

handle::handle(const Reference &ref_) : _ref(ref_) {}

void handle::override_reference(const Reference &ref_)
{
	_ref = ref_;
}

handle::operator Reference &()
{
	return _ref;
}

handle::operator const Reference &() const
{
	return _ref;
}

handle operation(OperationCode code, const Reference &a, const Reference &b, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Operation { code, a, b }, DebugInfo(loc))));
}

handle constant(const Constant &value, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(value, DebugInfo(loc))));
}

handle invocation(
	const SharedBlockReference &sbr,
	const std::vector <Reference> &args,
	const std::vector <Reference> &returns,
	const std::source_location &loc
) {
	return handle($tsb.add(Instruction(Invocation { sbr, args, returns }, DebugInfo(loc))));
}

handle construct(const Reference &type, const std::vector <Reference> &args, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Construct { type, args }, DebugInfo(loc))));
}

handle argument(const Reference &type, uint32_t argi, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Argument { type, argi }, DebugInfo(loc))));
}

handle returns(const Reference &type, uint32_t argi, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Return { type, argi }, DebugInfo(loc))));
}

handle stage_input(const Reference &type, uint32_t argi, RateProperties properties, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(StageInput { type, argi, properties }, DebugInfo(loc))));
}

handle stage_output(const Reference &type, uint32_t argi, RateProperties properties, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(StageOutput { type, argi, properties }, DebugInfo(loc))));
}

handle global_resource(const GlobalResource &resource, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(resource, DebugInfo(loc))));
}

handle system_value(SystemValue value, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(value, DebugInfo(loc))));
}

handle builtin_intrinsic(BuiltinIntrinsicCode code, const std::vector <Reference> &args, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(BuiltinIntrinsic { code, args }, DebugInfo(loc))));
}

handle swizzle(SwizzleCode code, const Reference &value, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Swizzle { code, value }, DebugInfo(loc))));
}

handle array_access(const Reference &value, const Reference &index, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(ArrayAccess { value, index }, DebugInfo(loc))));
}

handle field_access(const Reference &value, uint32_t fidx, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(FieldAccess { value, fidx }, DebugInfo(loc))));
}

void store(const Reference &destination, const Reference &source, const std::source_location &loc)
{
	$tsb.add(Instruction(Store { destination, source }, DebugInfo(loc)));
}

handle local(const Reference &type, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Local { type }, DebugInfo(loc))));
}

handle loop(const SharedBlockReference &body, const std::source_location &loc)
{
	return handle($tsb.add(Instruction(Loop { body }, DebugInfo(loc))));
}

handle type(const Type &t, const std::source_location &loc)
{
	auto key = t.repr();
	auto &cache = Tracer::singleton.type_cache;
	if (auto it = cache.find(key); it != cache.end()) {
		auto ref = it->second;
		auto &blk = Tracer::singleton.active();
		bool already_active = false;
		for (const auto &existing : blk) {
			if (existing == ref) {
				already_active = true;
				break;
			}
		}
		if (not already_active)
			blk.insert(blk.begin(), ref);
		return handle(ref);
	}
	auto ref = $tsb.add(Instruction(t, DebugInfo(loc)));
	cache.emplace(key, ref);
	return handle(ref);
}

} // namespace rcgp::jems
