#pragma once

#include <string>
#include "vulkan/vulkan.h"

class FVulkanDevice;
class FVulkanTexture;

class FVulkanTextureCreateInfo
{
public:
	std::string filename;
	VkFormat format;
	//VkImageUsageFlags imageUsageFlags;
	//VkImageLayout imageLayout;

	FVulkanTextureCreateInfo();
	VkResult Create(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue, FVulkanTexture& texture);
private:

	void CreateImage(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue, FVulkanTexture& texture);
	void CreateSampler(FVulkanDevice vulkanDevice, FVulkanTexture& texture);
	void CreateImageView(FVulkanDevice vulkanDevice, FVulkanTexture& texture);
};

