#pragma once

#include <map>

#include "resources.hpp"
#include "resources_collect.hpp"

namespace rcgp {

struct _stage_registry {
	static auto &get() {
		static std::map <void *, vk::ShaderStageFlags> map;
		return map;
	}
};

template <auto &ref, ShaderStage ... Ss>
void register_one_wrapper(const stage_wrapper <ref, Ss...> &)
{
	if constexpr (not is_push_constant_v <reference_base_of <ref>>)
		_stage_registry::get()[&ref] |= stage_flags_of <Ss...> ();
}

template <typename ... Ts>
void register_gvrs(const Tlist <Ts...> &)
{
	(register_one_wrapper(Ts {}), ...);
}

} // namespace rcgp
