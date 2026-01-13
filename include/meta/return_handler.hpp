#pragma once

#include <type_traits>

#include "../dsl/array.hpp"
#include "stage_intrinsics.hpp"

template <typename T>
void return_handler(const T &ret, size_t &argi) {}

template <reflected T>
void return_handler(const TaskPayload <T> &, size_t &)
{
	$tsb.context.task_payload_type = reconstruct_type <T> ();
}

// TODO: can we do ..., void>?
template <MeshPrimitive P, uint32_t MaxVertices, uint32_t MaxPrimitives, typename T>
requires std::is_void_v <T>
void return_handler(const MeshletPayload <P, MaxVertices, MaxPrimitives, T> &, size_t &)
{
	$tsb.context.mesh_max_vertices = MaxVertices;
	$tsb.context.mesh_max_primitives = MaxPrimitives;
	$tsb.context.mesh_primitive_kind = P;
}

template <MeshPrimitive P, uint32_t MaxVertices, uint32_t MaxPrimitives, reflected T>
void return_handler(const MeshletPayload <P, MaxVertices, MaxPrimitives, T> &, size_t &)
{
	$tsb.context.mesh_max_vertices = MaxVertices;
	$tsb.context.mesh_max_primitives = MaxPrimitives;
	$tsb.context.mesh_primitive_kind = P;
}

template <primitive T, RateProperties P>
void return_handler(const Interpolant <T, P> &ret, size_t &argi)
{
	auto type = reconstruct_type <T> ();
	auto tout = ThreadOutput(type, argi, P);
	$tsb.context.add_thread_output(tout);

	// Fix the argument index of the original value
	ret._ref->template as <ThreadOutput> ()
		.argi = argi++;
}

template <primitive T>
void return_handler(const T &ret, size_t &argi)
{
	auto type = reconstruct_type <T> ();
	auto tout = ThreadOutput(type, argi++, RateProperties::eNone);
	$tsb.context.add_thread_output(tout);

	jems::store(jems::thread_output(tout), ret);
}

template <typename ... Args>
void return_handler(const std::tuple <Args...> &ret, size_t &argi)
{
	constexpr_for(Is, sizeof...(Args),
		(return_handler(std::get <Is> (ret), argi), ...)
	);
}

// User-side interface with return mechanics
template <typename R>
struct _return_operator {};

template <typename R, typename U>
requires std::is_convertible_v <U, R>
void operator<<(const _return_operator <R>, U value)
{
	// Force conversion to get expected behavior; only
	// if necessary to avoid unexpected duplicate behaviour
	auto cvted = [&]() -> R {
		if constexpr (std::is_same_v <R, std::decay_t <U>>)
			return value;
		else
			return R(value);
	} ();

	size_t argi = 0;
	return_handler(cvted, argi);
}
