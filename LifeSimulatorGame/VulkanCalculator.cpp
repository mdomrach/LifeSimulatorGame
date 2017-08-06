#include "VulkanCalculator.h"
#include <stdexcept>

uint32_t FVulkanCalculator::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat FVulkanCalculator::FindDepthFormat(const VkPhysicalDevice physicalDevice)
{
	return FindSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat FVulkanCalculator::FindSupportedFormat(const VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat Format : candidates)
	{
		VkFormatProperties Props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, Format, &Props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (Props.linearTilingFeatures & features) == features)
		{
			return Format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (Props.optimalTilingFeatures & features) == features)
		{
			return Format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}