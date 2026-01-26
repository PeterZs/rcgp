#include "rhi/command_pool.hpp"

namespace rcgp {

CommandPool CommandPool::from(const Device &device, const Queue &queue)
{
	auto cpool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(queue.family_index)
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	return CommandPool(device.logical.createCommandPool(cpool_info));
}

} // namespace rcgp
