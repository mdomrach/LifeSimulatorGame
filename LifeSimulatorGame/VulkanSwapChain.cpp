#include "VulkanSwapChain.h"

void FVulkanSwapChain::SetImageCount(uint32_t newImageCount)
{
	imageCount = newImageCount;

	images.resize(imageCount);
	imageViews.resize(imageCount);
	frameBuffers.resize(imageCount);
}
