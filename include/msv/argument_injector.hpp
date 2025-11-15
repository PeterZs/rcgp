#pragma once

#include "../dsl/instructions.hpp"
#include "../dsl/jems.hpp"
#include "stage.hpp"
#include "static_string.hpp"
#include "context.hpp"

template <Stage S>
void inject_execution_model()
{
	if constexpr (S == Stage::RepresentationalVertex)
		$tsb.context.model = ExecutionModel::eVulkanVertex;
	else
		static_assert(false, "no execution model for stage");
}

template <typename T>
void inject_item(T &item, Reference ref)
{
	if constexpr (std::is_base_of_v <jems::handle, T>)
		item.ref = ref;
	else
		static_assert(false, ($ss("failed to inject reference into item of type ") + $ss_type(T)).view());
}

struct InjectionState {
	size_t argidx;
	size_t threadidx;

	struct Delta {
		bool argument;
		bool thread_input;
	};

	InjectionState next(Delta delta) const {
		return {
			.argidx = argidx + delta.argument,
			.threadidx = threadidx + delta.thread_input,
		};
	}
};

template <Stage S, typename T>
struct stage_argument_injector {};

// For subroutines, normals arguments are normal arguments
template <typename T>
struct stage_argument_injector <Stage::Undefined, T> {
	static auto apply(T &value, const InjectionState &state) {
		auto type = reconstruct_type <T> ();
		auto arg = Argument(type, state.argidx);
		$tsb.context.add_argument(arg);
		inject_item(value, jems::intrinsic(arg));
		return InjectionState::Delta {
			.argument = true,
			.thread_input = false,
		};
	}
};

// For vertex and fragment shaders, normal arguments are thread inputs
template <Stage S, typename T>
requires (S == Stage::RepresentationalVertex || S == Stage::RepresentationalFragment)
struct stage_argument_injector <S, T> {
	static auto apply(T &value, const InjectionState &state) {
		auto type = reconstruct_type <T> ();
		auto tin = ThreadInput(type, state.threadidx);
		$tsb.context.add_thread_input(tin);
		inject_item(value, jems::intrinsic(tin));
		return InjectionState::Delta {
			.argument = false,
			.thread_input = true,
		};
	}
};

// Always ignore the implicit context
template <Stage S, auto & ... refs>
struct stage_argument_injector <S, implicit_context <refs...>> {
	static auto apply(auto &value, const InjectionState &state) {
		return InjectionState::Delta { false, false };
	}
};

template <Stage S, size_t Index, typename ... Args>
void inject_arguments(std::tuple <Args...> &args, const InjectionState &state)
{
	auto &value = std::get <Index> (args);
	using T = std::decay_t <decltype(value)>;

	auto delta = stage_argument_injector <S, T> ::apply(value, state);
	if constexpr (Index + 1 < sizeof...(Args))
		inject_arguments <S, Index + 1> (args, state.next(delta));
}
