#pragma once

#include <variant>
#include <source_location>
#include <filesystem>

#include <fmt/format.h>

template <typename T, typename U, typename ... Args>
constexpr int variant_index(int i)
{
	if constexpr (std::same_as <T, U>)
		return i;
	if constexpr (sizeof...(Args))
		return variant_index <T, Args...> (i + 1);
	return -1;
}

template <typename ... Args>
struct variant : std::variant <Args...> {
	using std::variant <Args...> ::variant;

	template <typename T>
	bool is() {
		return std::holds_alternative <T> (*this);
	}
	
	template <typename T>
	T &as() {
		return std::get <T> (*this);
	}
	
	template <typename T>
	const T &as() const {
		return std::get <T> (*this);
	}

	template <typename T>
	static constexpr int type_index() {
		return variant_index <T, Args...> (0);
	}
};

#define vswitch(value)					\
	using T = std::decay_t <decltype(value)>;	\
	switch (value.index())

#define vcase(...) case T::type_index <__VA_ARGS__> ()

struct Instruction;

using Index = uint32_t;

// TODO: can either generate instructions 'statically' or 'persistently..'
// TODO: should we allow cross record instructions? index would then be (record id, local index)

using constant_base = variant <bool, int32_t, uint32_t, float, std::string>;

struct Constant : constant_base {
	using constant_base::variant;

	std::string str() {
		auto ftn = [](auto x) { return fmt::format("{}", x); };
		return std::visit(ftn, *this);
	}
};

struct Operation {
	enum {
		eAdd,
		eSubtract,
		eMultiply,
		eDivide,
	} code;

	Index a;
	Index b;

	std::string str() {
		std::string op = "?";
		switch (code) {
		case eAdd: op = "add"; break;
		default:
			break;
		}

		return fmt::format("{}(${}, ${})", op, a, b);
	}
};

template <typename T, size_t N>
struct VectorType {};

using primitive_base = variant <
	bool,
	int32_t,
	uint32_t,
	float,
	VectorType <uint32_t, 2>,
	VectorType <uint32_t, 3>,
	VectorType <uint32_t, 4>,
	VectorType <int32_t, 2>,
	VectorType <int32_t, 3>,
	VectorType <int32_t, 4>,
	VectorType <float, 2>,
	VectorType <float, 3>,
	VectorType <float, 4>
>;

struct PrimitiveType : primitive_base {
	using primitive_base::variant;

	std::string str() const {
		auto &value = *this;
		vswitch(value) {
		vcase(bool): return "bool";
		vcase(int32_t): return "i32";
		vcase(uint32_t): return "u32";
		vcase(float): return "f32";
		vcase(VectorType <float, 2>): return "vec2";
		vcase(VectorType <float, 3>): return "vec3";
		vcase(VectorType <float, 4>): return "vec4";
		default:
			break;
		}

		return "?";
	}
};

using type_base = variant <PrimitiveType>;

struct Type : type_base {
	using type_base::variant;

	std::string str() {
		auto ftn = [](auto x) { return x.str(); };
		return std::visit(ftn, *this);
	}
};

struct invocable {
	Index type;
	std::vector <Index> args;

	template <typename ... Args>
	invocable(Index type_, Args ... args_) : type(type_), args { args_ ... } {}
};

struct Construct : invocable {
	using invocable::invocable;

	std::string str() const {
		std::string result = fmt::format("new(type: ${}, args: ", type);
		for (size_t i = 0; i < args.size(); i++) {
			result += fmt::format("${}", args[i]);
			if (i + 1 < args.size())
				result += ", ";
		}
		return result + ")";
	}
};

struct Intrinsic {
	enum {
		eSVPosition,
	} code;

	// TODO: property methods, e.g. is_lvalue...

	std::string str() const {
		switch (code) {
		case eSVPosition: return "SVPosition";
		default:
			break;
		}
		
		return "?";
	}
};

struct Assign {
	Index dst;
	Index src;

	std::string str() {
		return fmt::format("${} = ${}", dst, src);
	}
};

// TODO: enable/disable with macros... or templated instructions?
struct Debug {
	std::source_location origin;
};

using instruction_base = variant <
	Constant,
	Operation,
	Type,
	Construct,
	Intrinsic,
	Assign
>;

struct Instruction : instruction_base {
	Debug debug_info;

	Instruction(const instruction_base &base, Debug debug_info_ = {})
		: instruction_base(base), debug_info(debug_info_) {}

	std::string str() const {
		auto ftn = [](auto x) { return x.str(); };
		return std::visit(ftn, *this);
	}
};

std::string format_as(const Instruction &instr)
{
	auto &loc = instr.debug_info.origin;
	auto rel = std::filesystem::relative(loc.file_name());
	return fmt::format("{}: {}:{}", instr.str(), rel.string(), loc.line());
}
