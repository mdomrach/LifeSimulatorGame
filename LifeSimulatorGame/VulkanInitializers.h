#pragma once

#include <vulkan/vulkan.h>

namespace FVulkanInitializers
{
	// device
	VkApplicationInfo ApplicationInfo();
	VkInstanceCreateInfo InstanceCreateInfo();
	VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCreateInfoEXT();
	VkDeviceQueueCreateInfo DeviceQueueCreateInfo();
	VkDeviceCreateInfo DeviceCreateInfo();

	VkSwapchainCreateInfoKHR SwapchainCreateInfoKHR();
	VkRenderPassCreateInfo RenderPassCreateInfo();
	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo();
	VkCommandPoolCreateInfo CommandPoolCreateInfo();

	// pipeline
	VkShaderModuleCreateInfo ShaderModuleCreateInfo();
	VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo();
	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo();
	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo();
	VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo();
	VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo();
	VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo();
	VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo();	// only used in TextOverlay
	VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo();
	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();
	VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo();

	// command buffer
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo();
	VkCommandBufferBeginInfo CommandBufferBeginInfo();
	VkSubmitInfo SubmitInfo();

	// image
	VkImageViewCreateInfo ImageViewCreateInfo(); // also used for preexisting images, i.e. swapchain
	VkImageCreateInfo ImageCreateInfo();
	VkMemoryAllocateInfo MemoryAllocateInfo();
	VkImageMemoryBarrier ImageMemoryBarrier();

	// Load scene assets
	VkSamplerCreateInfo SamplerCreateInfo();

	// Init prepare to display scene
	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo();
	VkFramebufferCreateInfo FramebufferCreateInfo();
	VkBufferCreateInfo BufferCreateInfo();
	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo();
	VkWriteDescriptorSet WriteDescriptorSet();
	VkPipelineCacheCreateInfo PipelineCacheCreateInfo();	// only used in TextOverlay
	VkRenderPassBeginInfo RenderPassBeginInfo();
	VkSemaphoreCreateInfo SemaphoreCreateInfo();

	// draw frame
	VkPresentInfoKHR PresentInfoKHR();
};