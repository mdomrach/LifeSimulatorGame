#pragma once

#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"
#include <vector>

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
	void CreateCommandBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, FVulkanSwapChain swapChain);
	void BuildCommandBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, FVulkanSwapChain swapChain, VkImage srcImage, VkQueue queue);
	void Submit(VkQueue queue, uint32_t bufferindex);
	void Destroy(FVulkanDevice vulkanDevice);

private:
	VkImage screenImage;
	VkDeviceMemory screenImageMemory;
	std::vector<VkCommandBuffer> cmdBuffers;
	VkSubresourceLayout subResourceLayout;

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
	void WriteDepthToFile(FVulkanDevice vulkanDevice, FVulkanSwapChain swapChain, const char* filename);
	void OutputCurrentMousePosDepth(FVulkanSwapChain swapChain);
	void MapMemory(FVulkanDevice vulkanDevice);
	FInputManager* inputManager;
	FVulkanApplication* vulkanApplication;

	bool isScreenShot;
	bool submitGrabDepth;
	bool writeDepthToFile;
	bool hasOutput;

	const char* data;
	float totalDepth = 0;
};
