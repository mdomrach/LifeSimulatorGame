#include "ShaderCalculator.h"
#include "VulkanInitializers.h"

VkShaderModule FShaderCalculator::CreateShaderModule(VkDevice logicalDevice, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = FVulkanInitializers::ShaderModuleCreateInfo();
	createInfo.codeSize = code.size();

	std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
	memcpy(codeAligned.data(), code.data(), code.size());
	createInfo.pCode = codeAligned.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}