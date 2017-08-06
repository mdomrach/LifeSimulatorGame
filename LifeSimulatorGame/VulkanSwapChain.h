#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class FVulkanSwapChain   
{
public:

	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	uint32_t imageCount;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	VkFormat colorFormat;

	// from tutorial
	// Allowed this to stay in swap chain
	VkExtent2D extent;

	// from examples
	// buffers contains copy of image views and frame buffers, confusing
	//std::vector<SwapChainBuffer> buffers;

	// VkFrameBuffer vs SwapChainBuffer?
	std::vector<VkFramebuffer> frameBuffers;

	void SetImageCount(uint32_t imageCount);
};