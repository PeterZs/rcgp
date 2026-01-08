#pragma once

#include <cstdint>
#include <optional>
#include <source_location>
#include <string>
#include <vector>

#include "instruction_enums.hpp"
#include "instruction_types.hpp"

struct Constant : variant <
	bool,
	int32_t,
	uint32_t,
	float, std::string
> {
	using variant_self::variant;
};

struct Operation {
	using Code = OperationCode;
	Code code;

	Reference a;
	Reference b;
};

struct Construct {
	Reference type;
	std::vector <Reference> args;

	template <typename ... Args>
	Construct(Reference type_, Args ... args_)
		: type(type_), args { args_ ... } {}
};

struct Invocation {
	SharedBlockReference sbr;
	std::vector <Reference> args;

	template <typename ... Args>
	Invocation(SharedBlockReference sbr_, Args ... args_)
		: sbr(sbr_), args { args_ ... } {}
};

struct Branch {
	struct Segment {
		Reference cond;
		SharedBlockReference body;
	};

	std::vector <Segment> segments;
	std::optional <SharedBlockReference> fallback;
};

struct Loop {
	LoopKind kind;
	std::optional <SharedBlockReference> init;
	SharedBlockReference cond;
	Reference cond_value;
	std::optional <SharedBlockReference> step;
	SharedBlockReference body;
};

struct Local {
	Reference type;
};

struct Argument {
	Reference type;
	uint32_t argi;
};

struct BuiltinIntrinsic {
	using Code = BuiltinIntrinsicCode;
	Code code;

	std::vector <Reference> args;
	
	template <typename ... Args>
	BuiltinIntrinsic(Code code_, Args ... args_)
		: code(code_), args { args_ ... } {}
};

struct Swizzle {
	using Code = SwizzleCode;
	Code code;

	Reference value;
};

std::string swizzle_string(SwizzleCode code);

struct FieldAccess {
	Reference value;
	uint32_t fidx;
};

struct ArrayAccess {
	Reference value;
	Reference index;
};

struct Store {
	Reference destination;
	Reference source;
};

// TODO: enable/disable with macros... or templated instructions?
struct Debug {
	std::source_location origin;
};

struct Return {
	Reference value;
};
