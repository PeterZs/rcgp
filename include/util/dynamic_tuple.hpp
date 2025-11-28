#pragma once

#include <cstddef>
#include <vector>
#include <type_traits>

#include "align.hpp"
#include "trivial_tuple.hpp"
#include "sequence.hpp"

template <typename ... Ts>
class dynamic_tuple;

template <typename T, typename ... Ts>
class dynamic_tuple <T[], Ts...> {
	using statics_t = trivial_tuple <Ts...>;

	static constexpr size_t static_count = sizeof...(Ts);
	static constexpr size_t dynamic_index = static_count;
	
	[[no_unique_address]] statics_t statics_storage;
	std::vector <T> dynamics_storage;

	static constexpr size_t statics_size() {
		if constexpr (sizeof...(Ts))
			return sizeof(statics_t);
		else
			return 0;
	}
public:
	using element_type = T;

	dynamic_tuple() = default;

	template <size_t Index>
	auto &get() {
		static_assert(Index <= static_count, "dynamic_tuple index out of range");

		if constexpr (Index == dynamic_index)
			return dynamics_storage;
		else
			return statics_storage.template get <Index> ();
	}

	template <size_t Index>
	const auto &get() const {
		static_assert(Index <= static_count, "dynamic_tuple index out of range");

		if constexpr (Index == dynamic_index)
			return dynamics_storage;
		else
			return statics_storage.template get <Index> ();
	}

	// Multi-level indices
	template <size_t Index, size_t ... Rest>
	auto &get_recursive() {
		auto &value = get <Index> ();
		if constexpr (sizeof...(Rest))
			return value.template get <Rest...> ();
		else
			return value;
	}

	template <size_t Index, size_t ... Rest>
	const auto &get_recursive() const {
		const auto &value = get <Index> ();
		if constexpr (sizeof...(Rest))
			return value.template get <Rest...> ();
		else
			return value;
	}

	template <size_t Index>
	static consteval size_t offset() {
		static_assert(Index <= static_count, "dynamic_tuple offset out of range");

		if constexpr (Index == dynamic_index)
			return dynamic_offset();
		else
			return statics_t::template offset <Index> ();
	}

	static constexpr size_t dynamic_offset() {
		return align_up(statics_size(), alignof(T));
	}

	static constexpr size_t size(size_t elements) {
		return dynamic_offset() + sizeof(T) * elements;
	}

	std::vector <T> &dynamic() {
		return dynamics_storage;
	}

	const std::vector <T> &dynamic() const {
		return dynamics_storage;
	}

	const statics_t &statics() const {
		return statics_storage;
	}

	statics_t &statics() {
		return statics_storage;
	}
};

template <typename T, typename ... Ts>
auto new_dynamic_tuple(std::type_identity <T>, sequence <Ts...>) -> dynamic_tuple <T, Ts...>;
