#pragma once

#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"

class FVulkanDevice;
class FVulkanSwapChain;
class FGameManager;
class FInputManager;
class FVulkanApplication;

class FVulkanScreenGrab
{
public:


	void ProcessInput();
	void Initialize(FGameManager* GameManager);

private:

	void InsertImageMemoryBarrier(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange);
	void GrabScreen(FVulkanDevice vulkanDevice, FVulkanSwapChain swapChain, VkCommandPool commandPool, VkQueue queue, VkImage sourceImage, const char* filename);
	void GrabDepth(FVulkanDevice vulkanDevice, FVulkanSwapChain swapChain, VkCommandPool commandPool, VkQueue queue, VkImage sourceImage, const char* filename);

	FInputManager* inputManager;
	FVulkanApplication* vulkanApplication;

	bool isScreenShot;
};

