#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace FShaderCalculator
{
	VkShaderModule CreateShaderModule(VkDevice logicalDevice, const std::vector<char>& code);
};

