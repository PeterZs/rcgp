#pragma once

#include <cstdint>
#include <optional>
#include <source_location>
#include <string>
#include <vector>
#include <print>
#include <memory>

#include <fmt/format.h>

#include "../util/variant.hpp"
#include "enumerations.hpp"

namespace rcgp {

// TODO: move repr implementations to instruction_nodes.cpp

// Forward declarations
struct Block;
struct Instruction;

using SharedBlockReference = std::shared_ptr <Block>;
using Reference = std::shared_ptr <Instruction>;

template <typename T, size_t N>
struct VectorType {};

template <typename T, size_t N, size_t M>
struct MatrixType {};

// TODO: just make this an enum...
struct PrimitiveType : variant <
	bool,
	int32_t,
	uint32_t,
	float,
	
	// Vector types
	VectorType <uint32_t, 2>,
	VectorType <uint32_t, 3>,
	VectorType <uint32_t, 4>,

	VectorType <int32_t, 2>,
	VectorType <int32_t, 3>,
	VectorType <int32_t, 4>,

	VectorType <float, 2>,
	VectorType <float, 3>,
	VectorType <float, 4>,

	// Matrix types
	MatrixType <int32_t, 2, 2>,
	MatrixType <int32_t, 3, 3>,
	MatrixType <int32_t, 4, 4>,
	MatrixType <uint32_t, 2, 2>,
	MatrixType <uint32_t, 3, 3>,
	MatrixType <uint32_t, 4, 4>,
	MatrixType <float, 2, 2>,
	MatrixType <float, 3, 3>,
	MatrixType <float, 4, 4>
> {
	using variant_self::variant;
};

struct AggregateType : std::vector <Reference> {
	std::string name;
};

struct ArrayType {
	Reference base;
	int64_t size;
};

// @node Type
struct Type : variant <
	PrimitiveType,
	AggregateType,
	ArrayType
> {
	using variant_self::variant;

	std::string repr() const;
};

// @node Constant
struct Constant : variant <
	bool,
	int32_t,
	uint32_t,
	float,
	std::string
> {
	using variant_self::variant;

	std::string repr() const {
		return std::visit([](auto x) {
			return fmt::format("{}", x);
		}, *this);
	}
};

// @node Operation
struct Operation {
	OperationCode code;
	Reference a;
	Reference b;

	std::string repr() const {
		return fmt::format("Operation({})", rcgp::repr(code));
	}
};

struct Construct {
	Reference type;
	std::vector <Reference> args;

	template <typename ... Args>
	Construct(Reference type_, Args ... args_)
		: type(type_), args { args_ ... } {}

	std::string repr() const {
		return "Construct";
	}
};

struct Invocation {
	SharedBlockReference sbr;
	std::vector <Reference> args;

	template <typename ... Args>
	Invocation(SharedBlockReference sbr_, Args ... args_)
		: sbr(sbr_), args { args_ ... } {}

	std::string repr() const {
		return "Invocation";
	}
};

struct Branch {
	struct Segment {
		Reference cond;
		SharedBlockReference body;
	};

	std::vector <Segment> segments;
	std::optional <SharedBlockReference> fallback;

	std::string repr() const {
		return "Branch";
	}
};

struct Loop {
	LoopKind kind;
	std::optional <SharedBlockReference> init;
	SharedBlockReference cond;
	Reference cond_value;
	std::optional <SharedBlockReference> step;
	SharedBlockReference body;

	std::string repr() const {
		return "Loop";
	}
};

struct Local {
	Reference type;

	std::string repr() const {
		return "Local";
	}
};

struct BuiltinIntrinsic {
	BuiltinIntrinsicCode code;

	std::vector <Reference> args;
	
	template <typename ... Args>
	BuiltinIntrinsic(BuiltinIntrinsicCode code_, Args ... args_)
		: code(code_), args { args_ ... } {}

	std::string repr() const {
		return std::format("BuiltinIntrinsic({})", rcgp::repr(code));
	}
};

struct Swizzle {
	SwizzleCode code;
	Reference value;

	std::string repr() const {
		return std::format("Swizzle({})", rcgp::repr(code));
	}
};

struct FieldAccess {
	Reference value;
	uint32_t fidx;

	std::string repr() const {
		return "FieldAccess";
	}
};

struct ArrayAccess {
	Reference value;
	Reference index;

	std::string repr() const {
		return "ArrayAccess";
	}
};

struct Store {
	Reference destination;
	Reference source;

	std::string repr() const {
		return "Store";
	}
};

struct GlobalResource {
	Reference type;
	GlobalResourceKind kind;
	GlobalResourceLayout layout;
	GlobalResourceAccess access = GlobalResourceAccess::eReadWrite;

	std::optional <uint32_t> group;
	std::optional <uint32_t> index;
	std::optional <uint32_t> offset;

	std::string repr() const {
		return std::format("GlobalResource({}, {})",
			rcgp::repr(kind), rcgp::repr(layout));
	}
};

struct Argument {
	Reference type;
	uint32_t argi;

	std::string repr() const {
		return std::format("Argument({})", argi);
	}
};

struct StageInput {
	Reference type;
	uint32_t argi;

	std::string repr() const {
		return std::format("StageInput({})", argi);
	}
};

struct StageOutput {
	Reference type;
	uint32_t argi;
	RateProperties properties;

	std::string repr() const {
		return std::format("StageOutput({}, {})", argi, rcgp::repr(properties));
	}
};

} // namespace rcgp
