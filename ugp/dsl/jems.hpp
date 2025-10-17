#pragma once

#include "tracer.hpp"
#include "instructions.hpp"

// JIT Emitters
namespace jems {

struct handle {
	// TODO: make this optional...
	Index index;

	handle(Index index_) : index(index_) {}

	// TODO: baseline operator=, then other structs use handle::operator=

	// TODO: make Index a struct to avoid implicit casting to int (for other types...)
	operator Index() {
		// TODO: err if no value
		return index;
	}
};

struct scope {
	scope(Tracer::Record &record) {
		Tracer::singleton.records.emplace(std::ref(record));
	}

	~scope() {
		Tracer::singleton.records.pop();
	}

	operator bool() const {
		return true;
	}
};

#define JEM(name, type)	\
	template <typename ... Args>	\
	struct name : handle {	\
		name(Args ... args, const std::source_location &loc = std::source_location::current())	\
			: handle(Tracer::singleton.active().add(type(args...), Debug(loc))) {}	\
	};	\
	template <typename ... Args>	\
	struct name##_loc : handle {	\
		name##_loc (const std::source_location &loc, Args ... args) \
			: handle(Tracer::singleton.active().add(type(args...), Debug(loc))) {}	\
	};	\
	template <typename ... Args>	\
	name(Args ...) -> name <Args...>;

JEM(operation,	Operation	);
JEM(constant,	Constant	);
JEM(type,	Type		);
JEM(construct,	Construct	);
JEM(intrinsic,	Intrinsic	);
JEM(assign,	Assign		);

} // namespace jems

#define $location const std::source_location &loc = std::source_location::current()
