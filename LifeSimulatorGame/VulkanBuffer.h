#pragma once

#include "vulkan/vulkan.h"

class FVulkanBuffer
{
public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;

	void Destroy(VkDevice device);
};

