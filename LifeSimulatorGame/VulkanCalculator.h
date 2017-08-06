#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace FVulkanCalculator
{
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat FindDepthFormat(const VkPhysicalDevice physicalDevice);
	VkFormat FindSupportedFormat(const VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};

