#include "TextOverlay.h"
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
#include "TimeManager.h"

void FTextOverlay::Initialize(FVulkanApplication* vulkanApplication, FVulkanDevice vulkanDevice)
{
	// Load the text rendering shaders

	auto vertShaderCode = FFileCalculator::ReadFile("shaders/text.vert.spv");
	auto fragShaderCode = FFileCalculator::ReadFile("shaders/text.frag.spv");

	VkShaderModule vertShaderModule = FShaderCalculator::CreateShaderModule(vulkanDevice.logicalDevice, vertShaderCode);
	VkShaderModule fragShaderModule = FShaderCalculator::CreateShaderModule(vulkanDevice.logicalDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> textshaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	InitializeHelper(vulkanApplication, textshaderStages);

	UpdateTextOverlay(vulkanDevice);

	vkDestroyShaderModule(vulkanDevice.logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(vulkanDevice.logicalDevice, fragShaderModule, nullptr);
}

void FTextOverlay::InitializeHelper(FVulkanApplication* application, std::vector<VkPipelineShaderStageCreateInfo> shaderStages)
{
	frameCount = 0;
	this->timeManager = application->timeManager;

	this->shaderStages = shaderStages;
	this->frameBufferHeight = &application->swapChain.extent.height;
	this->frameBufferWidth = &application->swapChain.extent.width;
	
	this->frameBuffers.resize(application->swapChain.frameBuffers.size());
	for (uint32_t i = 0; i < application->swapChain.frameBuffers.size(); i++)
	{
		this->frameBuffers[i] = &application->swapChain.frameBuffers[i];
	}

	cmdBuffers.resize(application->swapChain.frameBuffers.size());

	PrepareResources(application);
	PrepareRenderPass(application);
	PreparePipeline(application);
}

void FTextOverlay::Destroy(FVulkanDevice* vulkanDevice)
{
	// Free up all Vulkan resources requested by the text overlay
	vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
	vkDestroyImage(vulkanDevice->logicalDevice, image, nullptr);
	vkDestroyImageView(vulkanDevice->logicalDevice, imageView, nullptr); 
	vertexBuffer.Destroy(vulkanDevice->logicalDevice);
	vkFreeMemory(vulkanDevice->logicalDevice, imageMemory, nullptr);
	vkDestroyDescriptorSetLayout(vulkanDevice->logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(vulkanDevice->logicalDevice, descriptorPool, nullptr);
	vkDestroyPipelineLayout(vulkanDevice->logicalDevice, pipelineLayout, nullptr);
	vkDestroyPipelineCache(vulkanDevice->logicalDevice, pipelineCache, nullptr);
	vkDestroyPipeline(vulkanDevice->logicalDevice, pipeline, nullptr);
	vkDestroyRenderPass(vulkanDevice->logicalDevice, renderPass, nullptr);
	vkDestroyCommandPool(vulkanDevice->logicalDevice, commandPool, nullptr);
}


void FTextOverlay::CreateCommandPool(FVulkanDevice vulkanDevice)
{
	VkCommandPoolCreateInfo poolInfo = FVulkanInitializers::CommandPoolCreateInfo();
	poolInfo.queueFamilyIndex = vulkanDevice.queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(vulkanDevice.logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}
void FTextOverlay::CreateCommandBuffer(FVulkanDevice vulkanDevice)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)cmdBuffers.size();

	if (vkAllocateCommandBuffers(vulkanDevice.logicalDevice, &commandBufferAllocateInfo, cmdBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void FTextOverlay::CreateVertexBuffer(FVulkanDevice vulkanDevice)
{
	FBufferCreateInfo bufferInfo = {};
	bufferInfo.buffersize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
	bufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	if (bufferInfo.Create(vulkanDevice, vertexBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vulkan buffer!");
	}
}

void FTextOverlay::CreateFontTexture(FVulkanDevice vulkanDevice, VkQueue graphicsQueue)
{
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


	if (vkCreateImage(vulkanDevice.logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vulkanDevice.logicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = FVulkanInitializers::MemoryAllocateInfo();
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FVulkanCalculator::FindMemoryType(vulkanDevice.physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(vulkanDevice.logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory");
	}

	vkBindImageMemory(vulkanDevice.logicalDevice, image, imageMemory, 0);

	// staging buffer
	FBufferCreateInfo stagingbufferInfo = {};
	stagingbufferInfo.buffersize = allocInfo.allocationSize;
	stagingbufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingbufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingbufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	FVulkanBuffer stagingBuffer;
	stagingbufferInfo.Create(vulkanDevice, stagingBuffer);

	uint32_t *data;
	vkMapMemory(vulkanDevice.logicalDevice, stagingBuffer.bufferMemory, 0, allocInfo.allocationSize, 0, (void**)&data);
	memcpy(data, &font24pixels[0][0], STB_FONT_WIDTH * STB_FONT_HEIGHT);
	vkUnmapMemory(vulkanDevice.logicalDevice, stagingBuffer.bufferMemory);

	// copy to image
	VkCommandBuffer copyCommandBuffer = FVulkanCommandBufferCalculator::BeginSingleTimeCommands(vulkanDevice.logicalDevice, commandPool);

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

	FVulkanCommandBufferCalculator::EndSingleTimeCommands(copyCommandBuffer, vulkanDevice.logicalDevice, graphicsQueue, commandPool);
	stagingBuffer.Destroy(vulkanDevice.logicalDevice);

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

	if (vkCreateImageView(vulkanDevice.logicalDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
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


	if (vkCreateSampler(vulkanDevice.logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}


void FTextOverlay::CreateDescriptorPool(FVulkanDevice vulkanDevice)
{
	std::array<VkDescriptorPoolSize, 1> poolSizes;
	poolSizes[0] = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = FVulkanInitializers::DescriptorPoolCreateInfo();
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(vulkanDevice.logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
void FTextOverlay::CreateDescriptorSetLayout(FVulkanDevice vulkanDevice)
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

	if (vkCreateDescriptorSetLayout(vulkanDevice.logicalDevice, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}
void FTextOverlay::CreatePipelineLayout(FVulkanDevice vulkanDevice)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = FVulkanInitializers::PipelineLayoutCreateInfo();
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(vulkanDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
void FTextOverlay::CreateDescriptorSet(FVulkanDevice vulkanDevice)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = FVulkanInitializers::DescriptorSetAllocateInfo();
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(vulkanDevice.logicalDevice, &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS)
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

	vkUpdateDescriptorSets(vulkanDevice.logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}
void FTextOverlay::CreatePipelineCache(FVulkanDevice vulkanDevice)
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = FVulkanInitializers::PipelineCacheCreateInfo();

	if (vkCreatePipelineCache(vulkanDevice.logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline cache!");
	}
}
void FTextOverlay::PrepareResources(FVulkanApplication* application)
{
	auto vulkanDevice = application->vulkanDevice;
	
	CreateCommandPool(vulkanDevice);
	CreateCommandBuffer(vulkanDevice);
	CreateVertexBuffer(vulkanDevice);
	CreateFontTexture(vulkanDevice, application->graphicsQueue);
	CreateDescriptorPool(vulkanDevice);
	CreateDescriptorSetLayout(vulkanDevice);
	CreatePipelineLayout(vulkanDevice);
	CreateDescriptorSet(vulkanDevice);
	CreatePipelineCache(vulkanDevice);
}

void FTextOverlay::PrepareRenderPass(FVulkanApplication* application)
{
	auto* vulkanDevice = &application->vulkanDevice;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = application->swapChain.colorFormat;
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

void FTextOverlay::PreparePipeline(FVulkanApplication* application)
{
	auto* vulkanDevice = &application->vulkanDevice;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = FVulkanInitializers::PipelineInputAssemblyStateCreateInfo();
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	inputAssemblyState.flags = 0;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;


	VkPipelineRasterizationStateCreateInfo rasterizationState = FVulkanInitializers::PipelineRasterizationStateCreateInfo();
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;


	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.blendEnable = VK_TRUE;
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState = FVulkanInitializers::PipelineColorBlendStateCreateInfo();
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = FVulkanInitializers::PipelineDepthStencilStateCreateInfo();
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.front = depthStencilState.back;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	
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
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	if (vkCreateGraphicsPipelines(vulkanDevice->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void FTextOverlay::BeginTextUpdate(FVulkanDevice vulkanDevice)
{
	if (vkMapMemory(vulkanDevice.logicalDevice, vertexBuffer.bufferMemory, 0, VK_WHOLE_SIZE, 0, (void **)&mapped) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map memory!");
	}
	numLetters = 0;
}

void FTextOverlay::AddText(std::string text, float x, float y, ETextAlign align)
{
	if (mapped == nullptr)
	{
		throw std::runtime_error("nothing mapped!");
	}

	const float charW = 1.5f / *frameBufferWidth;
	const float charH = 1.5f / *frameBufferHeight;

	float fbW = (float)*frameBufferWidth;
	float fbH = (float)*frameBufferHeight;
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

void FTextOverlay::EndTextUpdate(FVulkanDevice vulkanDevice)
{
	vkUnmapMemory(vulkanDevice.logicalDevice, vertexBuffer.bufferMemory);
	mapped = nullptr;
	UpdateCommandBuffers();
}

void FTextOverlay::UpdateCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = FVulkanInitializers::CommandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = FVulkanInitializers::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = *frameBufferWidth;
	renderPassBeginInfo.renderArea.extent.height = *frameBufferHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < cmdBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = *frameBuffers[i];

		vkBeginCommandBuffer(cmdBuffers[i], &cmdBufferBeginInfo);

		vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = (float)*frameBufferWidth;
		viewport.height = (float)*frameBufferHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent.width = *frameBufferWidth;
		scissor.extent.height = *frameBufferHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);

		vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, &vertexBuffer.buffer, &offsets);
		vkCmdBindVertexBuffers(cmdBuffers[i], 1, 1, &vertexBuffer.buffer, &offsets);
		for (uint32_t j = 0; j < numLetters; j++)
		{
			vkCmdDraw(cmdBuffers[i], 4, 1, j * 4, 0);
		}


		vkCmdEndRenderPass(cmdBuffers[i]);

		if (vkEndCommandBuffer(cmdBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to end command buffer!");
		}
	}
}

void FTextOverlay::Submit(VkQueue queue, uint32_t bufferindex)
{
	if (!visible)
	{
		return;
	}

	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

void FTextOverlay::UpdateTextOverlay(FVulkanDevice vulkanDevice)
{
	BeginTextUpdate(vulkanDevice);

	int fps = frameCount;
	std::string timeText = std::to_string(fps);
	AddText(timeText, 5.0f, 5.0f, FTextOverlay::alignLeft);

	EndTextUpdate(vulkanDevice);
}



void FTextOverlay::UpdateFrame(FVulkanDevice vulkanDevice)
{
	frameCount++;
	if (timeManager->startFrameTime > nextFPSUpdateTime)
	{
		UpdateTextOverlay(vulkanDevice);
		frameCount = 0;
		nextFPSUpdateTime++;
	}
}