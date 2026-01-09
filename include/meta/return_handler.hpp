#pragma once

#include "stage_intrinsics.hpp"

template <typename T>
void return_handler(const T &ret, size_t &argi) {}

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
