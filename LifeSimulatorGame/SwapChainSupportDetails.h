#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct FSwapChainSupportDetails
{
public:
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

