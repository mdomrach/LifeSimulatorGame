#pragma once

#include <vulkan/vulkan.h>

class FVulkanDevice;

namespace FVulkanImageCalculator
{
	void CreateImage(FVulkanDevice vulkanDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	bool HasStencilComponent(VkFormat format);
	void CopyImageToBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};

