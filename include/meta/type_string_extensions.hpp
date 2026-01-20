#pragma once

#include "reflection.hpp"
#include "static_string.hpp"
#include "type_hash.hpp"

// Helpers for stringifying reflection data
template <size_t Indentation, bool Newline, size_t I, typename T, typename ... Args>
consteval auto enumerated_type_strings()
{
	constexpr auto main = static_string <Indentation> ('\t')
		+ $ss_ulong_decimal(I) + ": "_ss + $ss_type_indented(T, Indentation)
		+ static_string <Newline> ('\n');

	if constexpr (sizeof...(Args)) {
		constexpr auto rest = enumerated_type_strings <
			Indentation,
			Newline,
			I + 1,
			Args...
		> ();

		return main + rest;
	} else {
		return main;
	}
}

template <size_t Indentation, bool Newline, size_t I, typename Original, typename T, typename ... Args>
consteval auto enumerated_field_strings()
{
	constexpr auto main = static_string <Indentation> ('\t')
		+ $field_name(Original, I) + ": "_ss + $ss_type_indented(T, Indentation)
		+ static_string <Newline> ('\n');

	if constexpr (sizeof...(Args)) {
		constexpr auto rest = enumerated_field_strings <
			Indentation,
			Newline,
			I + 1,
			Original,
			Args...
		> ();

		return main + rest;
	} else {
		return main;
	}
}

// For sequences and tuples
template <typename ... Ts>
struct type_string <sequence <Ts...>> {
	template <size_t I>
	static consteval auto eval() {
		constexpr auto tabs = static_string <I> ('\t');
		return "sequence {\n"_ss
			+ enumerated_type_strings <I + 1, true, 0, Ts...> ()
			+ tabs + "}"_ss;
	}
};

template <typename ... Ts>
struct type_string <std::tuple <Ts...>> {
	template <size_t I>
	static consteval auto eval() {
		constexpr auto tabs = static_string <I> ('\t');
		return "std::tuple {\n"_ss
			+ enumerated_type_strings <I + 1, true, 0, Ts...> ()
			+ tabs + "}"_ss;
	}
};

// Overload name generator for reflection types
template <typename Original, typename ... Args>
struct type_string <aggregate_reflection <Original, Args...>> {
	template <size_t I>
	static consteval auto eval() {
		constexpr auto tabs = static_string <I> ('\t');
		return $ss_type_indented(Original, I) + " {\n"_ss
			+ enumerated_field_strings <I + 1, true, 0, Original, Args...> ()
			+ tabs + "}"_ss;
	}
};

template <typename T>
struct type_string <resource_group_reflection <T>> {
	template <size_t I>
	static consteval auto eval() {
		return "resource group of "_ss + $ss_type_indented(T, I);
	}
};

template <auto &ref, typename T>
struct type_string <reference_reflection <ref, T>> {
	struct inner {};

	template <size_t I>
	static consteval auto eval() {
		constexpr size_t id = 0;
		return "reference (hash: "_ss
			+ $ss_ulong_hex(type_hash_v <inner>)
			+ ") to "_ss
			+ $ss_type_indented(T, I);
	}
};

template <typename R, typename ... Args>
struct type_string <function_reflection <R, Args...>> {
	template <size_t I>
	static consteval auto eval() {
		constexpr auto tabs = static_string <I> ('\t');
		return "function {\n"_ss
			+ tabs + "\treturn: "_ss + $ss_type_indented(R, I + 1) + "\n"_ss
			+ tabs + "\targs: {\n"_ss
			+ enumerated_type_strings <I + 2, true, 0, Args...> ()
			+ tabs + "\t}\n"_ss
			+ tabs + "}"_ss;
	}
};
