#pragma once

#include <vulkan/vulkan.h>

class FVulkanDevice;

namespace FVulkanBufferCalculator
{
	void CreateBuffer(FVulkanDevice vulkanDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
};

