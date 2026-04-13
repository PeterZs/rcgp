#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "command_buffer.hpp"
#include "queue.hpp"

namespace rcgp {

struct CommandPool : vk::CommandPool {
	static CommandPool from(const Device &device, const Queue &queue);
};

template <typename F>
auto Device::one_shot(const Queue &queue, const CommandPool &cpool, F &&ftn) const
{
	constexpr bool is_invocable = std::is_invocable_v <F, const CommandBuffer &>;
	static_assert(is_invocable, "Device::one_shot: callable must be invocable with (const CommandBuffer &)");

	auto cmd = new_command_buffers(cpool, 1).front();

	using R = std::invoke_result_t <F, const CommandBuffer &>;
	using RV = std::conditional_t <std::is_same_v <R, void>, int, R>;

	RV x;

	cmd.begin();

	if constexpr (not std::is_same_v <R, void>)
		x = ftn(cmd);
	else
		ftn(cmd);

	cmd.end();

	queue.submit(cmd, {}, {}, vk::PipelineStageFlagBits::eBottomOfPipe, nullptr);
	queue.waitIdle();

	if constexpr (not std::is_same_v <R, void>)
		return x;
}

} // namespace rcgp
