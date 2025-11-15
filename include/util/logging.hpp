#pragma once

#include <fmt/printf.h>
#include <fmt/color.h>

template <typename ... Ts>
[[noreturn]] void fatal(fmt::format_string <Ts...> fmt_str, Ts &&...args)
{
	// TODO: source location...
	auto header = fmt::format(fmt::emphasis::bold, "ugp:");
	auto alert = fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::medium_purple), "fatal error:");
	auto message = fmt::format(fmt_str, std::forward <Ts> (args)...);
	fmt::println(stderr, "{} {} {}", header, alert, message);
	__builtin_trap();
}
