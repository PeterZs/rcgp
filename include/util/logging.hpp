#pragma once

#include <cstdarg>
#include <source_location>

namespace rcgp {

void info(const char *fmt_str, ...);
void warning(const char *fmt_str, ...);
void error(const char *fmt_str, ...);
void ok(const char *fmt_str, ...);

void assertion_impl(
	bool condition,
	const std::source_location &loc,
	const char *fmt_str,
	...
);
[[noreturn]] void fatal_impl(
	const std::source_location &loc,
	const char *fmt_str,
	...
);

#define assertion(cond, fmt_str, ...) \
	assertion_impl((cond), std::source_location::current(), (fmt_str) __VA_OPT__(,) __VA_ARGS__)

#define fatal(fmt_str, ...) \
	fatal_impl(std::source_location::current(), (fmt_str) __VA_OPT__(,) __VA_ARGS__)

} // namespace rcgp
