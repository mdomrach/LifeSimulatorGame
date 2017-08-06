#pragma once
#include <vulkan/vulkan.h>

class FVulkanDevice;

class FVulkanTexture
{
public:
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;

	void Destroy(FVulkanDevice vulkanDevice);
};
