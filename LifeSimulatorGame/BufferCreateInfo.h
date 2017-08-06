#pragma once

#include <vulkan/vulkan.h>

class FVulkanDevice;
class FVulkanBuffer;

class FBufferCreateInfo
{
public:
	VkDeviceSize buffersize;
	VkBufferUsageFlags bufferUsageFlags;
	VkMemoryPropertyFlags memoryPropertyFlags;
	VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult Create(const FVulkanDevice& vulkanDevice, FVulkanBuffer& buffer);
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
};