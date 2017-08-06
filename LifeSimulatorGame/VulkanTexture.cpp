#include "VulkanTexture.h"
#include "VulkanDevice.h"

void FVulkanTexture::Destroy(FVulkanDevice vulkanDevice)
{
	vkDestroySampler(vulkanDevice.logicalDevice, sampler, nullptr);
	vkDestroyImageView(vulkanDevice.logicalDevice, imageView, nullptr);
	vkDestroyImage(vulkanDevice.logicalDevice, image, nullptr);
	vkFreeMemory(vulkanDevice.logicalDevice, imageMemory, nullptr);
}