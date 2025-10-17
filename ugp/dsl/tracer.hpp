#pragma once

#include <stack>
#include <variant>

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
};

struct Instruction;

using Index = uint32_t;

// TODO: can either generate instructions 'statically' or 'persistently..'
// TODO: should we allow cross record instructions? index would then be (record id, local index)

using Constant = variant <bool, int32_t, uint32_t, float, std::string>;

struct BinaryOperation {
	enum {
		eAdd,
		eSubtract,
		eMultiply,
		eDivide,
	} code;

	Index a;
	Index b;
};

#include <source_location>

// TODO: enable/disable with macros... or templated instructions?
struct Debug {
	std::source_location origin;
};

using instruction_base = variant <Constant, BinaryOperation>;

struct Instruction : instruction_base {
	Debug debug_info;

	Instruction(const instruction_base &base, Debug debug_info_ = {})
		: instruction_base(base), debug_info(debug_info_) {}
};

#include <fmt/format.h>

std::string format_as(const Instruction &instr)
{
	auto &loc = instr.debug_info.origin;
	return fmt::format("?: {}:{}:{}",
		loc.file_name(),
		loc.line(),
		loc.column());
}

using record_base = std::vector <Instruction>;

// NOTE: this tracer is not limited to just shaders...
// it also includes game engine stuff/code
#include <fmt/printf.h>

struct Tracer {
	struct Record : record_base {
		template <typename ... Args>
		Index add(Args ... args) {
			auto result = size();
			emplace_back(args...);
			return result;
		}

		Instruction &operator[](const Index &a) {
			return record_base::operator[](a);
		}
		
		const Instruction &operator[](const Index &a) const {
			return record_base::operator[](a);
		}
	};

	std::stack <std::reference_wrapper <Record>> records;

	struct RecordScope {
		Tracer &origin;

		RecordScope(Tracer &origin_, Record &record) : origin(origin_) {
			origin.records.emplace(std::ref(record));
		}

		~RecordScope() {
			origin.records.pop();
		}

		operator bool() {
			return true;
		}
	};

	RecordScope begin(Record &record) {
		return RecordScope(*this, record);
	}

	Record &active() {
		if (records.empty()) {
			fmt::println(stderr, "no active record");
			__builtin_trap();
		}

		return records.top();
	}

	// Generators
	// TODO: macrofy...
	Index constant(Constant value, std::source_location loc = std::source_location::current()) {
		return active().add(value, Debug(loc));
	}

	Index binary_operation(decltype(BinaryOperation::code) code, Index a, Index b, std::source_location loc = std::source_location::current()) {
		return active().add(BinaryOperation(code, a, b), Debug(loc));
	}

	// Singleton
	static thread_local Tracer singleton;
};

inline thread_local Tracer Tracer::singleton;

#define $dsl Tracer::singleton
// now we can do $jit.<whatever>
