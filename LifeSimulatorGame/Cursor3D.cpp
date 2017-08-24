#include "Cursor3D.h"
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
#include "UniformBufferObject.h"
#include "VulkanBufferCalculator.h"
#include "TerrainVertex.h"

void FVulkanCursor3D::Initialize(FGameManager* gameManager)
{
	applicationData = gameManager->applicationData;

	scene = gameManager->scene;
}

void FVulkanCursor3D::UpdateSwapChain()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;
	// Load the text rendering shaders

	auto vertShaderCode = FFileCalculator::ReadFile("shaders/terrain.vert.spv");
	auto fragShaderCode = FFileCalculator::ReadFile("shaders/terrain.frag.spv");
	//auto vertShaderCode = FFileCalculator::ReadFile("shaders/terrain.vert.spv");
	//auto fragShaderCode = FFileCalculator::ReadFile("shaders/terrain.frag.spv");

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
	CreateUniformBuffer();
	CreateIndexBuffer();
	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreatePipelineLayout();
	CreateDescriptorSet();
	CreatePipelineCache();
	CreateRenderPass();
	CreateGraphicsPipeline();

	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);

	UpdateCommandBuffers();
}

void FVulkanCursor3D::Destroy()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	vertexBuffer.Destroy(logicalDevice);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyPipelineCache(logicalDevice, pipelineCache, nullptr);
	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
}

#include "Scene.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
void FVulkanCursor3D::UpdateFrame()
{
	//static auto startTime = std::chrono::high_resolution_clock::now();

	//auto currentTime = std::chrono::high_resolution_clock::now();
	//float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

	FUniformBufferObject uniformBufferObject = {};
	//uniformBufferObject.model = glm::translate(glm::mat4(), scene->position);
	//uniformBufferObject.model = glm::mat4();
	uniformBufferObject.model = glm::translate(glm::mat4(), scene->position);


	//uniformBufferObject.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//uniformBufferObject.model = glm::rotate(glm::mat4(), startFrameTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//uniformBufferObject.model = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	uniformBufferObject.view = scene->camera->view;
	uniformBufferObject.proj = scene->camera->proj;
	uniformBufferObject.proj[1][1] *= -1;
	//uniformBufferObject.view = glm::mat4();
	//uniformBufferObject.proj = glm::mat4();

	void* data;
	vkMapMemory(applicationData->vulkanDevice.logicalDevice, uniformBuffer.bufferMemory, 0, sizeof(uniformBufferObject), 0, &data);
	memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
	vkUnmapMemory(applicationData->vulkanDevice.logicalDevice, uniformBuffer.bufferMemory);
}

void FVulkanCursor3D::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = FVulkanInitializers::CommandPoolCreateInfo();
	poolInfo.queueFamilyIndex = applicationData->vulkanDevice.queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(applicationData->vulkanDevice.logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}
void FVulkanCursor3D::CreateCommandBuffer()
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

void FVulkanCursor3D::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(FUniformBufferObject);
	VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, uniformBuffer.buffer, uniformBuffer.bufferMemory);
}

#include "Mesh.h"

void FVulkanCursor3D::CreateVertexBuffer()
{
	numLetters = 1;
	VkDeviceSize bufferSize = sizeof(scene->mesh->vertices[0]) * scene->mesh->vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(applicationData->vulkanDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, scene->mesh->vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(applicationData->vulkanDevice.logicalDevice, stagingBufferMemory);

	VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags vertexMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, vertexBufferUsageFlags, vertexMemoryPropertyFlags, vertexBuffer.buffer, vertexBuffer.bufferMemory);

	FVulkanBufferCalculator::CopyBuffer(applicationData->vulkanDevice.logicalDevice, commandPool, applicationData->graphicsQueue, stagingBuffer, vertexBuffer.buffer, bufferSize);

	vkDestroyBuffer(applicationData->vulkanDevice.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(applicationData->vulkanDevice.logicalDevice, stagingBufferMemory, nullptr);
}
//void FVulkanCursor3D::CreateVertexBuffer()
//{
//	FBufferCreateInfo bufferInfo = {};
//	bufferInfo.buffersize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
//	bufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//	bufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//
//	if (bufferInfo.Create(applicationData->vulkanDevice, vertexBuffer) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create vulkan buffer!");
//	}
//}

void FVulkanCursor3D::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(scene->mesh->indices[0]) * scene->mesh->indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(applicationData->vulkanDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, scene->mesh->indices.data(), (size_t)bufferSize);
	vkUnmapMemory(applicationData->vulkanDevice.logicalDevice, stagingBufferMemory);

	VkBufferUsageFlags indexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VkMemoryPropertyFlags indexMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, indexBufferUsageFlags, indexMemoryPropertyFlags, indexBuffer.buffer, indexBuffer.bufferMemory);

	FVulkanBufferCalculator::CopyBuffer(applicationData->vulkanDevice.logicalDevice, commandPool, applicationData->graphicsQueue, stagingBuffer, indexBuffer.buffer, bufferSize);

	vkDestroyBuffer(applicationData->vulkanDevice.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(applicationData->vulkanDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void FVulkanCursor3D::CreateFontTexture()
{
	static unsigned char font24pixels[STB_FONT_HEIGHT][STB_FONT_WIDTH];
	STB_FONT_NAME(stbFontData, font24pixels, STB_FONT_HEIGHT);
}


void FVulkanCursor3D::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 1> poolSizes;
	poolSizes[0] = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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

void FVulkanCursor3D::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uniformBufferObjectLayoutBinding = {};
	uniformBufferObjectLayoutBinding.binding = 0;
	uniformBufferObjectLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferObjectLayoutBinding.descriptorCount = 1;
	uniformBufferObjectLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uniformBufferObjectLayoutBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uniformBufferObjectLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfo = FVulkanInitializers::DescriptorSetLayoutCreateInfo();
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(applicationData->vulkanDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void FVulkanCursor3D::CreatePipelineLayout()
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
void FVulkanCursor3D::CreateDescriptorSet()
{
	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = FVulkanInitializers::DescriptorSetAllocateInfo();
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(applicationData->vulkanDevice.logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(FUniformBufferObject);

	std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
	descriptorWrites[0] = FVulkanInitializers::WriteDescriptorSet();
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(applicationData->vulkanDevice.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
void FVulkanCursor3D::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = FVulkanInitializers::PipelineCacheCreateInfo();

	if (vkCreatePipelineCache(applicationData->vulkanDevice.logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline cache!");
	}
}


void FVulkanCursor3D::CreateRenderPass()
{
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
	depthAttachment.format = FVulkanCalculator::FindDepthFormat(applicationData->vulkanDevice.physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;
	subPass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = FVulkanInitializers::RenderPassCreateInfo();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(applicationData->vulkanDevice.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}


//void FVulkanCursor3D::CreateRenderPass()
//{
//	auto* vulkanDevice = &applicationData->vulkanDevice;
//
//	VkAttachmentDescription colorAttachment = {};
//	colorAttachment.format = applicationData->swapChain.colorFormat;
//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	//colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//	VkAttachmentDescription depthAttachment = {};
//	depthAttachment.format = FVulkanCalculator::FindDepthFormat(vulkanDevice->physicalDevice);
//	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	//depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//	VkAttachmentReference colorAttachmentRef = {};
//	colorAttachmentRef.attachment = 0;
//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//	VkAttachmentReference depthAttachmentRef = {};
//	depthAttachmentRef.attachment = 1;
//	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//	// Use subpass dependencies for image layout transitions
//	VkSubpassDependency subpassDependencies[2] = {};
//
//	VkSubpassDependency dependency = {};
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependency.dstSubpass = 0;
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.srcAccessMask = 0;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
//	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//	subpassDependencies[0].dstSubpass = 0;
//	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	// Transition from initial to final
//	subpassDependencies[1].srcSubpass = 0;
//	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	VkSubpassDescription subPassDescription = {};
//	subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subPassDescription.flags = 0;
//	subPassDescription.inputAttachmentCount = 0;
//	subPassDescription.pInputAttachments = NULL;
//	subPassDescription.colorAttachmentCount = 1;
//	subPassDescription.pColorAttachments = &colorAttachmentRef;
//	subPassDescription.pResolveAttachments = NULL;
//	subPassDescription.pDepthStencilAttachment = &depthAttachmentRef;
//	subPassDescription.preserveAttachmentCount = 0;
//	subPassDescription.pPreserveAttachments = NULL;
//
//	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
//	VkRenderPassCreateInfo renderPassInfo = FVulkanInitializers::RenderPassCreateInfo();
//	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//	renderPassInfo.pAttachments = attachments.data();
//	renderPassInfo.subpassCount = 1;
//	renderPassInfo.pSubpasses = &subPassDescription;
//	renderPassInfo.dependencyCount = 2;
//	renderPassInfo.pDependencies = subpassDependencies;
//
//	if (vkCreateRenderPass(vulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create render pass!");
//	}
//}

void FVulkanCursor3D::CreateGraphicsPipeline()
{
	auto* vulkanDevice = &applicationData->vulkanDevice;

	VkPipelineRasterizationStateCreateInfo rasterizationState = FVulkanInitializers::PipelineRasterizationStateCreateInfo();
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	VkViewport* viewPort = new VkViewport();
	viewPort->x = 0.0f;
	viewPort->y = 0.0f;
	viewPort->width = (float)applicationData->swapChain.extent.width;
	viewPort->height = (float)applicationData->swapChain.extent.height;
	viewPort->minDepth = 0.0f;
	viewPort->maxDepth = 1.0f;

	VkRect2D* scissor = new VkRect2D();
	scissor->offset = { 0, 0 };
	scissor->extent = applicationData->swapChain.extent;

	VkPipelineViewportStateCreateInfo viewPortState = FVulkanInitializers::PipelineViewportStateCreateInfo();
	viewPortState.viewportCount = 1;
	viewPortState.pViewports = viewPort;
	viewPortState.scissorCount = 1;
	viewPortState.pScissors = scissor;

	VkPipelineMultisampleStateCreateInfo multisampleState = FVulkanInitializers::PipelineMultisampleStateCreateInfo();
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.flags = 0;

	auto vertexBindingDescription = FTerrainVertex::GetVertexBindingDescription();
	auto vertexAttributeDescriptions = FTerrainVertex::GetVertexAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = FVulkanInitializers::PipelineVertexInputStateCreateInfo();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = FVulkanInitializers::GraphicsPipelineCreateInfo();
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;


	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = CreatePipelineInputAssemblyStateCreateInfo();
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = CreatePipelineColorBlendStateCreateInfo();
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewPortState;
	pipelineCreateInfo.pDepthStencilState = CreatePipelineDepthStencilStateCreateInfo();
	pipelineCreateInfo.pDynamicState = nullptr;
	//pipelineCreateInfo.pDynamicState = &dynamicState;
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

VkPipelineInputAssemblyStateCreateInfo* FVulkanCursor3D::CreatePipelineInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState = new VkPipelineInputAssemblyStateCreateInfo();
	inputAssemblyState->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState->flags = 0;
	inputAssemblyState->primitiveRestartEnable = VK_FALSE;
	return inputAssemblyState;
}

VkPipelineColorBlendStateCreateInfo* FVulkanCursor3D::CreatePipelineColorBlendStateCreateInfo()
{
	VkPipelineColorBlendAttachmentState* colorBlendAttachmentState = new VkPipelineColorBlendAttachmentState();
	colorBlendAttachmentState->blendEnable = VK_FALSE;
	colorBlendAttachmentState->blendEnable = VK_TRUE;
	colorBlendAttachmentState->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT;

	colorBlendAttachmentState->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState->colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState->alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo* colorBlendState = new VkPipelineColorBlendStateCreateInfo();
	colorBlendState->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState->logicOpEnable = VK_FALSE;
	colorBlendState->logicOp = VK_LOGIC_OP_COPY;
	colorBlendState->attachmentCount = 1;
	colorBlendState->pAttachments = colorBlendAttachmentState;
	colorBlendState->blendConstants[0] = 0.0f;
	colorBlendState->blendConstants[1] = 0.0f;
	colorBlendState->blendConstants[2] = 0.0f;
	colorBlendState->blendConstants[3] = 0.0f;
	return colorBlendState;
}

// COMPARE OFF?
VkPipelineDepthStencilStateCreateInfo* FVulkanCursor3D::CreatePipelineDepthStencilStateCreateInfo()
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

//VkPipelineDepthStencilStateCreateInfo* FVulkanCursor3D::CreatePipelineDepthStencilStateCreateInfo()
//{
//	VkPipelineDepthStencilStateCreateInfo* depthStencil = new VkPipelineDepthStencilStateCreateInfo();
//	depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//	depthStencil->depthTestEnable = VK_TRUE;
//	depthStencil->depthWriteEnable = VK_TRUE;
//	depthStencil->depthCompareOp = VK_COMPARE_OP_LESS;
//	depthStencil->depthBoundsTestEnable = VK_FALSE;
//	depthStencil->minDepthBounds = 0.0f; // Optional
//	depthStencil->maxDepthBounds = 1.0f; // Optional
//	depthStencil->stencilTestEnable = VK_FALSE;
//	depthStencil->front = {};
//	depthStencil->back = {};
//	return depthStencil;
//}

void FVulkanCursor3D::UpdateCommandBuffers()
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

	//VkViewport viewport{};
	//viewport.width = (float)frameBufferWidth;
	//viewport.height = (float)frameBufferHeight;
	//viewport.minDepth = 0.0f;
	//viewport.maxDepth = 1.0f;

	//VkRect2D scissor{};
	//scissor.extent.width = frameBufferWidth;
	//scissor.extent.height = frameBufferHeight;
	//scissor.offset.x = 0;
	//scissor.offset.y = 0;

	for (int32_t i = 0; i < commandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = applicationData->swapChain.frameBuffers[i];

		vkBeginCommandBuffer(commandBuffers[i], &cmdBufferBeginInfo);

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		//vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer.buffer, &offsets);

		//vkCmdDraw(commandBuffers[i], 4, 1, 0, 0);

		//vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		//vkCmdDraw(commandBuffers[i], 3, 1, 3, 0);

		//vkCmdDraw(commandBuffers[i], 3, 1, 1, 0);

		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		//vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(scene->mesh->indices.size()), 1, 0, 0, 0);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(scene->mesh->indices.size()), 1, 0, 0, 0);



		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to end command buffer!");
		}
	}
}

void FVulkanCursor3D::Submit(VkQueue graphicsQueue, uint32_t bufferindex)
{
	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[bufferindex];

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);


	//VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();

	//VkSemaphore waitSemaphores[] = { applicationData->imageAvailableSemaphore };
	//VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	//submitInfo.waitSemaphoreCount = 1;
	//submitInfo.pWaitSemaphores = waitSemaphores;
	//submitInfo.pWaitDstStageMask = waitStages;

	//submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = &commandBuffers[bufferindex];

	//VkSemaphore signalSemaphores[] = { applicationData->renderFinishedSemaphore };
	//submitInfo.signalSemaphoreCount = 1;
	//submitInfo.pSignalSemaphores = signalSemaphores;

	//if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to submit draw command buffer!");
	//}
}