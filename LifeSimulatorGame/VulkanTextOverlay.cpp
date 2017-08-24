#include "VulkanTextOverlay.h"
#include "VulkanDevice.h"
#include "VulkanApplication.h"
#include <vector>
#include "BufferCreateInfo.h"
#include "VulkanTools.h"
#include "VulkanInitializers.h"
#include "VulkanCalculator.h"
#include "VulkanCommandBufferCalculator.h"
#include "FileCalculator.h"
#include "ShaderCalculator.h"
#include "GameManager.h"
#include "VulkanApplicationData.h"
#include "TextOverlay.h"

void FVulkanTextOverlay::Initialize(FGameManager* gameManager)
{
	applicationData = gameManager->applicationData;
	this->gameManager = gameManager;
}

void FVulkanTextOverlay::UpdateSwapChain()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;
	// Load the text rendering shaders

	auto vertShaderCode = FFileCalculator::ReadFile("shaders/text.vert.spv");
	auto fragShaderCode = FFileCalculator::ReadFile("shaders/text.frag.spv");

	VkShaderModule vertShaderModule = FShaderCalculator::CreateShaderModule(logicalDevice, vertShaderCode);
	VkShaderModule fragShaderModule = FShaderCalculator::CreateShaderModule(logicalDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> textshaderStages = { vertShaderStageInfo, fragShaderStageInfo };
	
	this->shaderStages = textshaderStages;

	CreateCommandPool();
	CreateCommandBuffer();
	CreateVertexBuffer();
	CreateFontTexture();
	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreatePipelineLayout();
	CreateDescriptorSet();
	CreatePipelineCache();
	CreateRenderPass();
	CreateGraphicsPipeline();

	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
}

void FVulkanTextOverlay::Destroy()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	// Free up all Vulkan resources requested by the text overlay
	vkDestroySampler(logicalDevice, sampler, nullptr);
	vkDestroyImage(logicalDevice, image, nullptr);
	vkDestroyImageView(logicalDevice, imageView, nullptr);
	vertexBuffer.Destroy(logicalDevice);
	vkFreeMemory(logicalDevice, imageMemory, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyPipelineCache(logicalDevice, pipelineCache, nullptr);
	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
}


void FVulkanTextOverlay::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = FVulkanInitializers::CommandPoolCreateInfo();
	poolInfo.queueFamilyIndex = applicationData->vulkanDevice.queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(applicationData->vulkanDevice.logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}
void FVulkanTextOverlay::CreateCommandBuffer()
{
	commandBuffers.resize(applicationData->swapChain.imageCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(applicationData->vulkanDevice.logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void FVulkanTextOverlay::CreateVertexBuffer()
{
	FBufferCreateInfo bufferInfo = {};
	bufferInfo.buffersize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
	bufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	if (bufferInfo.Create(applicationData->vulkanDevice, vertexBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vulkan buffer!");
	}
}

void FVulkanTextOverlay::CreateFontTexture()
{
	auto physicalDevice = applicationData->vulkanDevice.physicalDevice;
	auto graphicsQueue = applicationData->graphicsQueue;
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	static unsigned char font24pixels[STB_FONT_HEIGHT][STB_FONT_WIDTH];
	STB_FONT_NAME(stbFontData, font24pixels, STB_FONT_HEIGHT);


	// font texture
	VkImageCreateInfo imageInfo = FVulkanInitializers::ImageCreateInfo();
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8_UNORM;
	imageInfo.extent.width = STB_FONT_WIDTH;
	imageInfo.extent.height = STB_FONT_HEIGHT;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


	if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(logicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = FVulkanInitializers::MemoryAllocateInfo();
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FVulkanCalculator::FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory");
	}

	vkBindImageMemory(logicalDevice, image, imageMemory, 0);

	// staging buffer
	FBufferCreateInfo stagingbufferInfo = {};
	stagingbufferInfo.buffersize = allocInfo.allocationSize;
	stagingbufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingbufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingbufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	FVulkanBuffer stagingBuffer;
	stagingbufferInfo.Create(applicationData->vulkanDevice, stagingBuffer);

	uint32_t *data;
	vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, allocInfo.allocationSize, 0, (void**)&data);
	memcpy(data, &font24pixels[0][0], STB_FONT_WIDTH * STB_FONT_HEIGHT);
	vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

	// copy to image
	VkCommandBuffer copyCommandBuffer = FVulkanCommandBufferCalculator::BeginSingleTimeCommands(logicalDevice, commandPool);

	// prepare for transfer
	FVulkanTools::SetImageLayout(
		copyCommandBuffer,
		image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent = { STB_FONT_WIDTH, STB_FONT_HEIGHT, 1 };

	vkCmdCopyBufferToImage(copyCommandBuffer, stagingBuffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	// prepare for shader read

	FVulkanTools::SetImageLayout(
		copyCommandBuffer,
		image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	FVulkanCommandBufferCalculator::EndSingleTimeCommands(copyCommandBuffer, logicalDevice, graphicsQueue, commandPool);
	stagingBuffer.Destroy(logicalDevice);

	VkImageViewCreateInfo imageViewInfo = FVulkanInitializers::ImageViewCreateInfo();
	imageViewInfo.image = image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = imageInfo.format;
	imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,	VK_COMPONENT_SWIZZLE_A };
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(logicalDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	// Sampler
	VkSamplerCreateInfo samplerInfo = FVulkanInitializers::SamplerCreateInfo();
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;


	if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}


void FVulkanTextOverlay::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 1> poolSizes;
	poolSizes[0] = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = FVulkanInitializers::DescriptorPoolCreateInfo();
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(applicationData->vulkanDevice.logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
void FVulkanTextOverlay::CreateDescriptorSetLayout()
{
	std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings;
	setLayoutBindings[0] = {};
	setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBindings[0].binding = 0;
	setLayoutBindings[0].descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = FVulkanInitializers::DescriptorSetLayoutCreateInfo();
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

	if (vkCreateDescriptorSetLayout(applicationData->vulkanDevice.logicalDevice, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}
void FVulkanTextOverlay::CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = FVulkanInitializers::PipelineLayoutCreateInfo();
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(applicationData->vulkanDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
void FVulkanTextOverlay::CreateDescriptorSet()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = FVulkanInitializers::DescriptorSetAllocateInfo();
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(applicationData->vulkanDevice.logicalDevice, &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorImageInfo texDescriptor = {};
	texDescriptor.sampler = sampler;
	texDescriptor.imageView = imageView;
	texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	std::array<VkWriteDescriptorSet, 1> writeDescriptorSets;
	writeDescriptorSets[0] = FVulkanInitializers::WriteDescriptorSet();
	writeDescriptorSets[0].dstSet = descriptorSet;
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].pImageInfo = &texDescriptor;

	vkUpdateDescriptorSets(applicationData->vulkanDevice.logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}
void FVulkanTextOverlay::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = FVulkanInitializers::PipelineCacheCreateInfo();

	if (vkCreatePipelineCache(applicationData->vulkanDevice.logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline cache!");
	}
}

void FVulkanTextOverlay::CreateRenderPass()
{
	auto* vulkanDevice = &applicationData->vulkanDevice;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = applicationData->swapChain.colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FVulkanCalculator::FindDepthFormat(vulkanDevice->physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Use subpass dependencies for image layout transitions
	VkSubpassDependency subpassDependencies[2] = {};

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subPassDescription = {};
	subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPassDescription.flags = 0;
	subPassDescription.inputAttachmentCount = 0;
	subPassDescription.pInputAttachments = NULL;
	subPassDescription.colorAttachmentCount = 1;
	subPassDescription.pColorAttachments = &colorAttachmentRef;
	subPassDescription.pResolveAttachments = NULL;
	subPassDescription.pDepthStencilAttachment = &depthAttachmentRef;
	subPassDescription.preserveAttachmentCount = 0;
	subPassDescription.pPreserveAttachments = NULL;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = FVulkanInitializers::RenderPassCreateInfo();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDescription;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = subpassDependencies;

	if (vkCreateRenderPass(vulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void FVulkanTextOverlay::CreateGraphicsPipeline()
{
	auto* vulkanDevice = &applicationData->vulkanDevice;

	VkPipelineRasterizationStateCreateInfo rasterizationState = FVulkanInitializers::PipelineRasterizationStateCreateInfo();
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;
	
	VkPipelineViewportStateCreateInfo viewportState = FVulkanInitializers::PipelineViewportStateCreateInfo();
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.flags = 0;


	VkPipelineMultisampleStateCreateInfo multisampleState = FVulkanInitializers::PipelineMultisampleStateCreateInfo();
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.flags = 0;

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = FVulkanInitializers::PipelineDynamicStateCreateInfo();
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicState.flags = 0;


	std::array<VkVertexInputBindingDescription, 2> vertexBindings = {};
	vertexBindings[0].binding = 0;
	vertexBindings[0].stride = sizeof(glm::vec4);
	vertexBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexBindings[1].binding = 1;
	vertexBindings[1].stride = sizeof(glm::vec4);
	vertexBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


	std::array<VkVertexInputAttributeDescription, 2> vertexAttribs = {};
	// Position
	vertexAttribs[0] = {};
	vertexAttribs[0].location = 0;
	vertexAttribs[0].binding = 0;
	vertexAttribs[0].format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttribs[0].offset = 0;

	// UV
	vertexAttribs[1] = {};
	vertexAttribs[1].location = 1;
	vertexAttribs[1].binding = 1;
	vertexAttribs[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttribs[1].offset = sizeof(glm::vec2);
		
	VkPipelineVertexInputStateCreateInfo inputState = FVulkanInitializers::PipelineVertexInputStateCreateInfo();
	inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
	inputState.pVertexBindingDescriptions = vertexBindings.data();
	inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
	inputState.pVertexAttributeDescriptions = vertexAttribs.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = FVulkanInitializers::GraphicsPipelineCreateInfo() ;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;



	pipelineCreateInfo.pVertexInputState = &inputState;
	pipelineCreateInfo.pInputAssemblyState = CreatePipelineInputAssemblyStateCreateInfo();
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = CreatePipelineColorBlendStateCreateInfo();
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = CreatePipelineDepthStencilStateCreateInfo();
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	if (vkCreateGraphicsPipelines(vulkanDevice->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	delete pipelineCreateInfo.pInputAssemblyState;
	delete pipelineCreateInfo.pColorBlendState->pAttachments;
	delete pipelineCreateInfo.pColorBlendState;
	delete pipelineCreateInfo.pDepthStencilState;
}

VkPipelineInputAssemblyStateCreateInfo* FVulkanTextOverlay::CreatePipelineInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState = new VkPipelineInputAssemblyStateCreateInfo();
	inputAssemblyState->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	inputAssemblyState->flags = 0;
	inputAssemblyState->primitiveRestartEnable = VK_FALSE;
	return inputAssemblyState;
}

VkPipelineColorBlendStateCreateInfo* FVulkanTextOverlay::CreatePipelineColorBlendStateCreateInfo()
{
	VkPipelineColorBlendAttachmentState* colorBlendAttachmentState = new VkPipelineColorBlendAttachmentState();
	colorBlendAttachmentState->blendEnable = VK_TRUE;
	colorBlendAttachmentState->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState->dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState->colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState->dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment->blendEnable = VK_FALSE;
	//colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState->alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo* colorBlendState = new VkPipelineColorBlendStateCreateInfo();
	colorBlendState->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState->attachmentCount = 1;
	colorBlendState->pAttachments = colorBlendAttachmentState;
	//colorBlending->logicOpEnable = VK_FALSE;
	//colorBlending->logicOp = VK_LOGIC_OP_COPY;
	//colorBlending->blendConstants[0] = 0.0f;
	//colorBlending->blendConstants[1] = 0.0f;
	//colorBlending->blendConstants[2] = 0.0f;
	//colorBlending->blendConstants[3] = 0.0f;
	return colorBlendState;
}

VkPipelineDepthStencilStateCreateInfo* FVulkanTextOverlay::CreatePipelineDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo* depthStencilState = new VkPipelineDepthStencilStateCreateInfo();
	depthStencilState->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState->depthTestEnable = VK_TRUE;
	depthStencilState->depthWriteEnable = VK_TRUE;
	depthStencilState->depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	//depthStencil->depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState->depthBoundsTestEnable = VK_FALSE;
	depthStencilState->minDepthBounds = 0.0f; // Optional
	depthStencilState->maxDepthBounds = 1.0f; // Optional
	depthStencilState->stencilTestEnable = VK_FALSE;
	depthStencilState->front = {};
	depthStencilState->back = {};
	//depthStencilState.front = depthStencilState.back;
	//depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	return depthStencilState;
}



void FVulkanTextOverlay::BeginTextUpdate(VkDevice logicalDevice)
{
	if (vkMapMemory(logicalDevice, vertexBuffer.bufferMemory, 0, VK_WHOLE_SIZE, 0, (void **)&mapped) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map memory!");
	}
	numLetters = 0;
}

void FVulkanTextOverlay::AddText(std::string text, float x, float y, ETextAlign align)
{
	if (mapped == nullptr)
	{
		throw std::runtime_error("nothing mapped!");
	}

	auto frameBufferWidth = applicationData->swapChain.extent.width;
	auto frameBufferHeight = applicationData->swapChain.extent.height;

	const float charW = 1.5f / frameBufferWidth;
	const float charH = 1.5f / frameBufferHeight;

	float fbW = (float)frameBufferWidth;
	float fbH = (float)frameBufferHeight;
	x = (x / fbW * 2.0f) - 1.0f;
	y = (y / fbH * 2.0f) - 1.0f;

	// Calculate text width
	float textWidth = 0;
	for (auto letter : text)
	{
		stb_fontchar *charData = &stbFontData[(uint32_t)letter - STB_FIRST_CHAR];
		textWidth += charData->advance * charW;
	}

	switch (align)
	{
	case alignRight:
		x -= textWidth;
		break;
	case alignCenter:
		x -= textWidth / 2.0f;
		break;
	}

	// Generate a uv mapped quad per char in the new text
	for (auto letter : text)
	{
		stb_fontchar *charData = &stbFontData[(uint32_t)letter - STB_FIRST_CHAR];

		mapped->x = (x + (float)charData->x0 * charW);
		mapped->y = (y + (float)charData->y0 * charH);
		mapped->z = charData->s0;
		mapped->w = charData->t0;
		mapped++;

		mapped->x = (x + (float)charData->x1 * charW);
		mapped->y = (y + (float)charData->y0 * charH);
		mapped->z = charData->s1;
		mapped->w = charData->t0;
		mapped++;

		mapped->x = (x + (float)charData->x0 * charW);
		mapped->y = (y + (float)charData->y1 * charH);
		mapped->z = charData->s0;
		mapped->w = charData->t1;
		mapped++;

		mapped->x = (x + (float)charData->x1 * charW);
		mapped->y = (y + (float)charData->y1 * charH);
		mapped->z = charData->s1;
		mapped->w = charData->t1;
		mapped++;

		x += charData->advance * charW;

		numLetters++;
	}
}

void FVulkanTextOverlay::EndTextUpdate(VkDevice logicalDevice)
{
	vkUnmapMemory(logicalDevice, vertexBuffer.bufferMemory);
	mapped = nullptr;
	UpdateCommandBuffers();
}

void FVulkanTextOverlay::UpdateCommandBuffers()
{
	auto frameBufferWidth = applicationData->swapChain.extent.width;
	auto frameBufferHeight = applicationData->swapChain.extent.height;

	VkCommandBufferBeginInfo cmdBufferBeginInfo = FVulkanInitializers::CommandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = FVulkanInitializers::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = frameBufferWidth;
	renderPassBeginInfo.renderArea.extent.height = frameBufferHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < commandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = applicationData->swapChain.frameBuffers[i];

		vkBeginCommandBuffer(commandBuffers[i], &cmdBufferBeginInfo);

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = (float)frameBufferWidth;
		viewport.height = (float)frameBufferHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent.width = frameBufferWidth;
		scissor.extent.height = frameBufferHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer.buffer, &offsets);
		vkCmdBindVertexBuffers(commandBuffers[i], 1, 1, &vertexBuffer.buffer, &offsets);
		for (uint32_t j = 0; j < numLetters; j++)
		{
			vkCmdDraw(commandBuffers[i], 4, 1, j * 4, 0);
		}


		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to end command buffer!");
		}
	}
}

void FVulkanTextOverlay::Submit(VkQueue queue, uint32_t bufferindex)
{
	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[bufferindex];

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

void FVulkanTextOverlay::UpdateTextOverlay()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;
	BeginTextUpdate(logicalDevice);
	for (int i = 0; i < gameManager->textOverlay.size(); i++)
	{
		auto textOverlay = gameManager->textOverlay[i];
		AddText(textOverlay->text, textOverlay->x, textOverlay->y, textOverlay->align);
	}
	EndTextUpdate(logicalDevice);
}
