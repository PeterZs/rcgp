#pragma once

#include <cstddef>
#include <utility>

#include "../../util/sequence.hpp"
#include "../../util/align.hpp"

// Padded elements
template <typename T, size_t N>
class padded_t : public T {
	[[no_unique_address]] char _pad[N];
public:
	padded_t() = default;
	padded_t(const T &value_) : T(value_) {}

	auto &operator=(const T &value_) {
		return T::operator=(value_);
	}

	operator T &() & {
		return static_cast <T &> (*this);
	}

	operator const T &() const {
		return static_cast <T &> (*this);
	}
};

template <typename T, size_t Extent, size_t Pad>
class padded_t <T[Extent], Pad> {
	T value[Extent];
	[[no_unique_address]] char _pad[Pad];
public:
	padded_t() = default;

	auto &operator[](size_t index) {
		return value[index];
	}

	const auto &operator[](size_t index) const {
		return value[index];
	}

	using array_type = T[Extent];

	operator array_type &() {
		return value;
	}

	operator const array_type &() const {
		return value;
	}
};

template <typename T, size_t N>
requires std::is_fundamental_v <T>
class padded_t <T, N> {
	// Need to manually place field
	T value;
	[[no_unique_address]] char _pad[N];
public:
	padded_t() = default;
	padded_t(const T &value_) : value(value_) {}

	padded_t &operator=(const T &value_) {
		value = value_;
		return *this;
	}

	operator T &() & {
		return value;
	}

	operator const T &() const {
		return value;
	}
};

// Translating arrays to index sequences at compile-time
template <auto &A, std::size_t ... Is>
constexpr auto array_to_index_sequence_impl(std::index_sequence <Is...>)
{
	return std::index_sequence <A[Is]...> {};
}

template <auto &A>
using array_to_index_sequence = decltype(array_to_index_sequence_impl <A> (std::make_index_sequence <A.size()> {}));

// Associate fields with padding amounts
template <typename Fields, typename Padding>
struct layout_stitcher {
	using type = sequence <>;
};

template <typename T, typename ... Ts, size_t I, size_t ... Is>
struct layout_stitcher <sequence <T, Ts...>, std::index_sequence <I, Is...>> {
	using next = layout_stitcher <sequence <Ts...>, std::index_sequence <Is...>>;
	using type = next::type::template push_front_t <padded_t <T, I>>;
};
