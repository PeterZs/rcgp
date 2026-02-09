#pragma once

#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace rcgp {

struct Attachments {
	std::vector <vk::AttachmentDescription> descriptions;
	std::map <std::string, uint32_t> mapping;

	vk::AttachmentDescription &operator[](const std::string &key) &;

	vk::AttachmentReference reference(const std::string &key, vk::ImageLayout layout) const;
};

struct Subpass {
	std::vector <vk::AttachmentReference> colors;
	std::vector <vk::AttachmentReference> depths;
	std::vector <vk::AttachmentReference> resolves;
	std::vector <vk::AttachmentReference> inputs;
};

inline auto subpass(
	std::initializer_list <vk::AttachmentReference> colors,
	std::initializer_list <vk::AttachmentReference> depths = {},
	std::initializer_list <vk::AttachmentReference> resolves = {},
	std::initializer_list <vk::AttachmentReference> inputs = {}
) -> Subpass
{
	return Subpass {
		.colors = std::vector <vk::AttachmentReference> (colors),
		.depths = std::vector <vk::AttachmentReference> (depths),
		.resolves = std::vector <vk::AttachmentReference> (resolves),
		.inputs = std::vector <vk::AttachmentReference> (inputs),
	};
}

} // namespace rcgp
