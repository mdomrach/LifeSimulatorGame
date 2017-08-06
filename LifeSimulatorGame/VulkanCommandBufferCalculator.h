#pragma once
#include <vulkan/vulkan.h>

namespace FVulkanCommandBufferCalculator
{
	VkCommandBuffer BeginSingleTimeCommands(VkDevice logicalDevice, VkCommandPool commandPool);
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice logicalDevice, VkQueue queue, VkCommandPool commandPool);
};

