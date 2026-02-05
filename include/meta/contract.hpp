#pragma once

#include <type_traits>

#include "../util/cti.hpp"

namespace rcgp {

template <auto &ref>
using contract_base_t = std::decay_t <decltype(ref)>;

// TODO: should enforce that base is a resource type
template <auto &ref>
struct contract : contract_base_t <ref> {
	static inline auto address = &ref;
	static constexpr auto &handle = ref;
};

TYPE_TRAIT(is_contract);
	template <auto &ref>
	TYPE_TRAIT_INCLUDES(is_contract, contract <ref>);

} // namespace rcgp
