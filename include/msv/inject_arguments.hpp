#pragma once

#include "../dsl/instructions.hpp"
#include "../dsl/jems.hpp"
#include "context.hpp"
#include "reconstruct_type.hpp"
#include "stage.hpp"
#include "static_string.hpp"

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

template <typename T>
struct reference_reconstructor_t {
	static jems::handle main() {
		static_assert(false, ($ss("reference reconstructor for ") + $ss_type(T) + $ss(" has not been implemented yet")).view());
	}
};

template <typename T>
struct reference_reconstructor_t <parameter_block_reflection <T>> {
	static jems::handle main() {
		auto type = reconstruct_type <T> ();
		// TODO: allocation policy depends on consituents of the block...
		// if its all non resources then that fine, but we need to split
		// it otherwise... may be wise to split earlier...
		// TODO: for splits we may add a split_reflection holder...
		// OR we can do a subreference <ref, I> that gets generated
		// BEFORE we begin injection...
		// 
		// NOTE: the programming model would be simpler for both me and the user
		// if we restrict each parameter block to stick to its own descriptor set
		// then we just need to make a sequence for each resource...
		//
		// NOTE: policy is as follows. Collect each non-GRV leaf into a single aggregate
		// and leave it as unallocated; after full pipeline inspection, if no other block
		// has occupied the push constant location, then allocate it for this one; if its
		// too big for push constants then fallback to uniform buffers, and if its too big
		// for that too, then fallback to storage buffers...; all this happens at runtime
		// using the device-specific vendor limits
		//
		// TODO: next:
		// [x] expand reflection for nested aggregates
		// [ ] add basic resources like uniform and storage buffers
		auto reference = jems::intrinsic(GlobalResource(type, GlobalResource::eUnallocated));
		return reference;
	}
};

template <typename T>
jems::handle reconstruct_reference()
{
	using R = expand_reflection <T> ::type;
	return reference_reconstructor_t <R> ::main();
}

// For all stages, handling resource references is basically the same
template <Stage S, auto &ref>
struct stage_argument_injector <S, reference <ref>> {
	using T = std::decay_t <decltype(ref)>;

	static auto apply(reference <ref> &value, const InjectionState &state) {
		auto r = reconstruct_reference <T> ();

		// TODO: inject item...
		// inject_item(value, jems::intrinsic(tin));

		// TODO: need to register the reference into the block...

		return InjectionState::Delta {
			.argument = false,
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
