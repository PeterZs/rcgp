#pragma once

#include <source_location>

#include "instruction_nodes.hpp"

#define $location const std::source_location &loc = std::source_location::current()

namespace rcgp::jems {

struct scope {
	scope(const SharedBlockReference &sbr);
	~scope();

	operator bool() const;
};

struct null {
	void override_reference(const Reference &ref);
};

class handle {
protected:
	Reference _ref;
public:
	handle(const Reference &ref_ = nullptr);

	void override_reference(const Reference &ref_);

	operator Reference &();
	operator const Reference &() const;
};

handle operation(OperationCode code, const Reference &a, const Reference &b = {}, $location);
handle constant(const Constant &value, $location);
handle invocation(const SharedBlockReference &sbr, const std::vector <Reference> &args, const std::vector <Reference> &returns, $location);
handle construct(const Reference &type, const std::vector <Reference> &args, $location);
handle argument(const Reference &type, uint32_t argi, $location);
handle returns(const Reference &type, uint32_t argi, $location);
handle stage_input(const Reference &type, uint32_t argi, RateProperties properties = {}, $location);
handle stage_output(const Reference &type, uint32_t argi, RateProperties properties = {}, $location);
handle global_resource(const GlobalResource &resource, $location);
handle system_value(SystemValue value, $location);
handle builtin_intrinsic(BuiltinIntrinsicCode code, const std::vector <Reference> &args = {}, $location);
handle swizzle(SwizzleCode code, const Reference &value, $location);
handle array_access(const Reference &value, const Reference &index, $location);
handle field_access(const Reference &value, uint32_t fidx, $location);
void   store(const Reference &destination, const Reference &source, $location);
handle local(const Reference &type, $location);
handle loop(const SharedBlockReference &body, $location);
handle type(const Type &t, $location);

} // namespace rcgp::jems

// TODO: put somewhere else, like builtins?
#define $break ::rcgp::jems::builtin_intrinsic(::rcgp::BuiltinIntrinsicCode::eBreak)
#define $continue ::rcgp::jems::builtin_intrinsic(::rcgp::BuiltinIntrinsicCode::eContinue)
#define $discard ::rcgp::jems::builtin_intrinsic(::rcgp::BuiltinIntrinsicCode::eDiscard)
