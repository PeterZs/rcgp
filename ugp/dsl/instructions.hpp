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
	bool is() const {
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

// TODO: need a better (paged arena) allocator which effectively just uint32_t or smth
using Reference = std::shared_ptr <Instruction>;

// TODO: can either generate instructions 'statically' or 'persistently..'
// TODO: should we allow cross record instructions? index would then be (record id, local index)

using constant_base = variant <bool, int32_t, uint32_t, float, std::string>;

struct Constant : constant_base {
	using constant_base::variant;
};

struct Operation {
	enum : uint32_t {
		eAdd,
		eSubtract,
		eMultiply,
		eDivide,
	} code;

	Reference a;
	Reference b;
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
};

using type_base = variant <PrimitiveType>;

struct Type : type_base {
	using type_base::variant;
};

struct invocable {
	Reference type;
	std::vector <Reference> args;

	template <typename ... Args>
	invocable(Reference type_, Args ... args_) : type(type_), args { args_ ... } {}
};

struct Construct : invocable {
	using invocable::invocable;
};

struct Intrinsic {
	enum : uint32_t {
		eSVPosition,
	} code;

	// TODO: property methods, e.g. is_lvalue...
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

// TODO: this whole infrastructure can be reused for future compiler projects :)
struct Block : std::vector <Reference> {
	// TODO: parameters and type information...

	template <typename T>
	Reference add(const T &sub, Debug aux);
};

using instruction_base = variant <
	Constant,
	Operation,
	Type,
	Construct,
	Intrinsic,
	Store,
	Block,
	Reference
>;

struct Instruction : instruction_base {
	Debug debug_info;

	Instruction(const instruction_base &base, Debug debug_info_ = {})
		: instruction_base(base), debug_info(debug_info_) {}
};

template <typename T>
Reference Block::add(const T &sub, const Debug aux)
{
	auto result = std::make_shared <Instruction> (sub, aux);
	emplace_back(result);
	return result;
}
