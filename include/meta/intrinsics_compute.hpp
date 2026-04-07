#pragma once

#include "intrinsics_common.hpp"

namespace rcgp {

// TODO: should also support mesh, task...
using LocalInvocationID = read_only_intrinsic <SystemValue::eLocalInvocationID, ShaderStage::eCompute, uvec3>;
using WorkGroupID = read_only_intrinsic <SystemValue::eWorkGroupID, ShaderStage::eCompute, uvec3>;
using GlobalInvocationID = read_only_intrinsic <SystemValue::eGlobalInvocationID, ShaderStage::eCompute, uvec3>;

template <uint32_t X, uint32_t Y = 1, uint32_t Z = 1>
struct WorkGroup {
	static constexpr uint32_t size_x = X;
	static constexpr uint32_t size_y = Y;
	static constexpr uint32_t size_z = Z;
	LocalInvocationID local_index;
	WorkGroupID workgroup_index;
	GlobalInvocationID global_index;
};

TYPE_TRAIT(is_workgroup);
	template <uint32_t X, uint32_t Y, uint32_t Z>
	TYPE_TRAIT_INCLUDES(is_workgroup, WorkGroup <X, Y, Z>);

} // namespace rcgp
