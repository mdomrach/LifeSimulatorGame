#include "VulkanInitializers.h"

VkBufferCreateInfo FVulkanInitializers::BufferCreateInfo()
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	return bufferInfo;
}

VkMemoryAllocateInfo FVulkanInitializers::MemoryAllocateInfo()
{
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	return allocInfo;
}

VkCommandPoolCreateInfo FVulkanInitializers::CommandPoolCreateInfo()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	return poolInfo;
}

VkCommandBufferAllocateInfo FVulkanInitializers::CommandBufferAllocateInfo()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	return commandBufferAllocateInfo;
}

VkImageCreateInfo FVulkanInitializers::ImageCreateInfo()
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	return imageInfo;
}

VkImageViewCreateInfo FVulkanInitializers::ImageViewCreateInfo()
{
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	return imageViewInfo;
}

VkSamplerCreateInfo FVulkanInitializers::SamplerCreateInfo()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	return samplerInfo;
}

VkDescriptorPoolCreateInfo FVulkanInitializers::DescriptorPoolCreateInfo()
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	return descriptorPoolInfo;
}

VkDescriptorSetLayoutCreateInfo FVulkanInitializers::DescriptorSetLayoutCreateInfo()
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	return descriptorSetLayoutInfo;
}

VkPipelineLayoutCreateInfo FVulkanInitializers::PipelineLayoutCreateInfo()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	return pipelineLayoutInfo;
}

VkDescriptorSetAllocateInfo FVulkanInitializers::DescriptorSetAllocateInfo()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	return descriptorSetAllocInfo;
}

VkWriteDescriptorSet FVulkanInitializers::WriteDescriptorSet()
{
	VkWriteDescriptorSet writeDescriptorSets = {};
	writeDescriptorSets.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	return writeDescriptorSets;
}

VkPipelineCacheCreateInfo FVulkanInitializers::PipelineCacheCreateInfo()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	return pipelineCacheCreateInfo;
}

VkRenderPassCreateInfo FVulkanInitializers::RenderPassCreateInfo()
{
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	return renderPassInfo;
}

VkPipelineInputAssemblyStateCreateInfo FVulkanInitializers::PipelineInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	return inputAssemblyState;
}

VkPipelineRasterizationStateCreateInfo FVulkanInitializers::PipelineRasterizationStateCreateInfo()
{
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	return rasterizationState;
}



VkPipelineColorBlendStateCreateInfo FVulkanInitializers::PipelineColorBlendStateCreateInfo()
{
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	return colorBlendState;
}

VkPipelineDepthStencilStateCreateInfo FVulkanInitializers::PipelineDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	return depthStencilState;
}

VkPipelineViewportStateCreateInfo FVulkanInitializers::PipelineViewportStateCreateInfo()
{
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	return viewportState;
}

VkPipelineMultisampleStateCreateInfo FVulkanInitializers::PipelineMultisampleStateCreateInfo()
{
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	return multisampleState;
}

VkPipelineDynamicStateCreateInfo FVulkanInitializers::PipelineDynamicStateCreateInfo()
{
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	return dynamicState;
}

VkPipelineVertexInputStateCreateInfo FVulkanInitializers::PipelineVertexInputStateCreateInfo()
{
	VkPipelineVertexInputStateCreateInfo inputState{};
	inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	return inputState;
}

VkGraphicsPipelineCreateInfo FVulkanInitializers::GraphicsPipelineCreateInfo()
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	return pipelineCreateInfo;
}



VkCommandBufferBeginInfo FVulkanInitializers::CommandBufferBeginInfo()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	return cmdBufferBeginInfo;
}

VkRenderPassBeginInfo FVulkanInitializers::RenderPassBeginInfo()
{
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	return renderPassBeginInfo;
}

VkSubmitInfo FVulkanInitializers::SubmitInfo()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	return submitInfo;
}



VkApplicationInfo FVulkanInitializers::ApplicationInfo()
{
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	return applicationInfo;
}

VkInstanceCreateInfo FVulkanInitializers::InstanceCreateInfo()
{
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	return createInfo;
}

VkSwapchainCreateInfoKHR FVulkanInitializers::SwapchainCreateInfoKHR()
{
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	return createInfo;
}


VkPipelineShaderStageCreateInfo FVulkanInitializers::PipelineShaderStageCreateInfo()
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	return vertShaderStageInfo;
}

VkShaderModuleCreateInfo FVulkanInitializers::ShaderModuleCreateInfo()
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	return createInfo;
}

VkFramebufferCreateInfo FVulkanInitializers::FramebufferCreateInfo()
{
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	return frameBufferInfo;
}

VkPresentInfoKHR FVulkanInitializers::PresentInfoKHR()
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	return presentInfo;
}

VkSemaphoreCreateInfo FVulkanInitializers::SemaphoreCreateInfo()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	return semaphoreInfo;
}


VkImageMemoryBarrier FVulkanInitializers::ImageMemoryBarrier()
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	return barrier;
}


VkDebugReportCallbackCreateInfoEXT FVulkanInitializers::DebugReportCallbackCreateInfoEXT()
{
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	return createInfo;
}

VkDeviceQueueCreateInfo FVulkanInitializers::DeviceQueueCreateInfo()
{
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	return queueCreateInfo;
}

VkDeviceCreateInfo FVulkanInitializers::DeviceCreateInfo()
{
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	return createInfo;
}