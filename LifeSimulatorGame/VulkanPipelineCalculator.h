#pragma once

#include <vulkan/vulkan.h>

class FVulkanSwapChain;

namespace FVulkanPipelineCalculator
{
	VkGraphicsPipelineCreateInfo* CreateGraphicsPipelineInfo(FVulkanSwapChain swapChain, VkDescriptorSetLayout descriptorSetLayout, VkDevice logicalDevice, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
	void DeleteGraphicsPipelineInfo(VkGraphicsPipelineCreateInfo* info);
};

