#pragma once

#include <type_traits>

#include "../util/cti.hpp"

namespace rcgp {

template <auto &ref>
using reference_base_of = std::decay_t <decltype(ref)>;

template <auto &ref>
using contract_base_of = std::decay_t <decltype(ref)> ::handle_type;

template <auto &ref>
struct contract : contract_base_of <ref> {
	using reference_base = reference_base_of <ref>;
	using contract_base_of <ref> ::operator=;

	static inline auto address = &ref;
	static constexpr auto &handle = ref;
};

TYPE_TRAIT(is_contract);
	template <auto &ref>
	TYPE_TRAIT_INCLUDES(is_contract, contract <ref>);

} // namespace rcgp
