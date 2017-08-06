#pragma once
#include "vulkan/vulkan.h"

namespace FVulkanFactory
{
	VkImageView ImageView(VkDevice logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

