#include "Cursor3D.h"
#include "VulkanDevice.h"

#include "VulkanInitializers.h"
//#include <stdexcept>
#include "VulkanFactory.h"
#include "VulkanBufferCalculator.h"
#include "VulkanDevice.h"
#include "VulkanImageCalculator.h"
#include "Vertex.h"
#include "FileCalculator.h"
#include "ShaderCalculator.h"
#include "UniformBufferObject.h"
#include "Scene.h"
#include "Camera.h"
#include "Mesh.h"
#include "VulkanTextureCreateInfo.h"
#include "TerrainVertex.h"
#include "GameManager.h"
#include "VulkanApplication.h"
#include "VulkanCalculator.h"
#include "VulkanPipelineCalculator.h"
#include "VulkanApplicationData.h"


void FVulkanCursor3D::Initialize(FGameManager* gameManager)
{
	this->applicationData = gameManager->applicationData;
	this->scene = gameManager->scene;

	LoadAssets();
}

void FVulkanCursor3D::UpdateSwapChain()
{
	auto logicalDevice = applicationData->vulkanDevice;

	CreateCommandPool();
	CreateCommandBuffer();
	CreateBuffers();
	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreatePipelineLayout();
	CreateDescriptorSet();
	CreatePipelineCache();
	PrepareRenderPass();
	CreateGraphicsPipeline();

	UpdateCommandBuffers();
}

void FVulkanCursor3D::CreateBuffers()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffer();
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
	commandBuffers.resize(applicationData->swapChain.frameBuffers.size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	if (vkAllocateCommandBuffers(applicationData->vulkanDevice.logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void FVulkanCursor3D::CreateDescriptorPool()
{
	// Descriptor
	// Font uses a separate descriptor pool
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

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uniformBufferObjectLayoutBinding };
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = FVulkanInitializers::DescriptorSetLayoutCreateInfo();
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(applicationData->vulkanDevice.logicalDevice, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}
void FVulkanCursor3D::CreatePipelineLayout()
{
	// Pipeline layout
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
	// Pipeline cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = FVulkanInitializers::PipelineCacheCreateInfo();

	if (vkCreatePipelineCache(applicationData->vulkanDevice.logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline cache!");
	}
}

void FVulkanCursor3D::LoadAssets()
{
	auto mesh2 = new FMesh();
	mesh2->vertices = {
		{ { -5.0f, -5.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { 5.0f, -5.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 5.0f, 5.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -5.0f, 5.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } },

		{ { -5.0f, -5.0f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { 5.0f, -5.0f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 5.0f, 5.0f, -0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { -5.0f, 5.0f, -0.5f },{ 1.0f, 1.0f, 1.0f } }
	};

	mesh2->indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	scene->mesh2 = mesh2;
}

void FVulkanCursor3D::Destroy()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	// Free up all Vulkan resources requested by the text overlay
	uniformBuffer.Destroy(logicalDevice);
	indexBuffer.Destroy(logicalDevice);
	vertexBuffer.Destroy(logicalDevice);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyPipelineCache(logicalDevice, pipelineCache, nullptr);
	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

}

void FVulkanCursor3D::CreateGraphicsPipeline()
{	
	//VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = FVulkanInitializers::PipelineInputAssemblyStateCreateInfo();
	//inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	//inputAssemblyState.flags = 0;
	//inputAssemblyState.primitiveRestartEnable = VK_FALSE;


	//VkPipelineRasterizationStateCreateInfo rasterizationState = FVulkanInitializers::PipelineRasterizationStateCreateInfo();
	//rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	//rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	//rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//rasterizationState.flags = 0;
	//rasterizationState.depthClampEnable = VK_FALSE;
	//rasterizationState.lineWidth = 1.0f;


	//VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	//colorBlendAttachmentState.blendEnable = VK_TRUE;
	//colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT;
	//colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	//VkPipelineColorBlendStateCreateInfo colorBlendState = FVulkanInitializers::PipelineColorBlendStateCreateInfo();
	//colorBlendState.attachmentCount = 1;
	//colorBlendState.pAttachments = &colorBlendAttachmentState;

	//VkPipelineDepthStencilStateCreateInfo depthStencilState = FVulkanInitializers::PipelineDepthStencilStateCreateInfo();
	//depthStencilState.depthTestEnable = VK_TRUE;
	//depthStencilState.depthWriteEnable = VK_TRUE;
	//depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	//depthStencilState.front = depthStencilState.back;
	//depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	//VkPipelineViewportStateCreateInfo viewportState = FVulkanInitializers::PipelineViewportStateCreateInfo();
	//viewportState.viewportCount = 1;
	//viewportState.scissorCount = 1;
	//viewportState.flags = 0;


	//VkPipelineMultisampleStateCreateInfo multisampleState = FVulkanInitializers::PipelineMultisampleStateCreateInfo();
	//multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//multisampleState.flags = 0;

	//std::vector<VkDynamicState> dynamicStateEnables = {
	//	VK_DYNAMIC_STATE_VIEWPORT,
	//	VK_DYNAMIC_STATE_SCISSOR
	//};

	//VkPipelineDynamicStateCreateInfo dynamicState = FVulkanInitializers::PipelineDynamicStateCreateInfo();
	//dynamicState.pDynamicStates = dynamicStateEnables.data();
	//dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	//dynamicState.flags = 0;


	//std::array<VkVertexInputBindingDescription, 2> vertexBindings = {};
	//vertexBindings[0].binding = 0;
	//vertexBindings[0].stride = sizeof(glm::vec4);
	//vertexBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	//vertexBindings[1].binding = 1;
	//vertexBindings[1].stride = sizeof(glm::vec4);
	//vertexBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


	//std::array<VkVertexInputAttributeDescription, 2> vertexAttribs = {};
	//// Position
	//vertexAttribs[0] = {};
	//vertexAttribs[0].location = 0;
	//vertexAttribs[0].binding = 0;
	//vertexAttribs[0].format = VK_FORMAT_R32G32_SFLOAT;
	//vertexAttribs[0].offset = 0;

	//// UV
	//vertexAttribs[1] = {};
	//vertexAttribs[1].location = 1;
	//vertexAttribs[1].binding = 1;
	//vertexAttribs[1].format = VK_FORMAT_R32G32_SFLOAT;
	//vertexAttribs[1].offset = sizeof(glm::vec2);

	//VkPipelineVertexInputStateCreateInfo inputState = FVulkanInitializers::PipelineVertexInputStateCreateInfo();
	//inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
	//inputState.pVertexBindingDescriptions = vertexBindings.data();
	//inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
	//inputState.pVertexAttributeDescriptions = vertexAttribs.data();

	//VkGraphicsPipelineCreateInfo pipelineCreateInfo = FVulkanInitializers::GraphicsPipelineCreateInfo();
	//pipelineCreateInfo.layout = pipelineLayout;
	//pipelineCreateInfo.renderPass = renderPass;
	//pipelineCreateInfo.flags = 0;
	//pipelineCreateInfo.basePipelineIndex = -1;
	//pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;



	//pipelineCreateInfo.pVertexInputState = &inputState;
	//pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	//pipelineCreateInfo.pRasterizationState = &rasterizationState;
	//pipelineCreateInfo.pColorBlendState = &colorBlendState;
	//pipelineCreateInfo.pMultisampleState = &multisampleState;
	//pipelineCreateInfo.pViewportState = &viewportState;
	//pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	//pipelineCreateInfo.pDynamicState = &dynamicState;
	//pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	//pipelineCreateInfo.pStages = shaderStages.data();

	//if (vkCreateGraphicsPipelines(vulkanDevice->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to create graphics pipeline!");
	//}

	VkGraphicsPipelineCreateInfo* pipelineInfo = FVulkanPipelineCalculator::CreateGraphicsPipelineInfo(applicationData->swapChain, descriptorSetLayout, applicationData->vulkanDevice.logicalDevice, renderPass, pipelineLayout);

	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	pipelineInfo->pInputAssemblyState = CreatePipelineInputAssemblyStateCreateInfo();
	pipelineInfo->pDepthStencilState = CreatePipelineDepthStencilStateCreateInfo();
	pipelineInfo->pColorBlendState = CreatePipelineColorBlendStateCreateInfo();

	auto vertShaderCode = FFileCalculator::ReadFile("shaders/terrain.vert.spv");
	auto fragShaderCode = FFileCalculator::ReadFile("shaders/terrain.frag.spv");

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


	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	auto vertexBindingDescription = FTerrainVertex::GetVertexBindingDescription();
	auto vertexAttributeDescriptions = FTerrainVertex::GetVertexAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = FVulkanInitializers::PipelineVertexInputStateCreateInfo();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
	
	pipelineInfo->stageCount = 2;
	pipelineInfo->pStages = shaderStages.data();
	pipelineInfo->pVertexInputState = &vertexInputInfo;

	if (vkCreateGraphicsPipelines(logicalDevice, pipelineCache, 1, pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
	
	delete pipelineInfo->pInputAssemblyState;
	delete pipelineInfo->pDepthStencilState;
	delete pipelineInfo->pColorBlendState->pAttachments;
	delete pipelineInfo->pColorBlendState;
}

void FVulkanCursor3D::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(FUniformBufferObject);
	VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, uniformBuffer.buffer, uniformBuffer.bufferMemory);
}


void FVulkanCursor3D::UpdateFrame()
{
	UpdateUniformBuffer();
}


#include <glm/gtc/matrix_transform.hpp>
void FVulkanCursor3D::UpdateUniformBuffer()
{
	VkDevice logicalDevice = applicationData->vulkanDevice.logicalDevice;

	FUniformBufferObject uniformBufferObject = {};
	//uniformBufferObject.model = glm::translate(glm::mat4(), scene->position);
	uniformBufferObject.model = glm::mat4();
	
	uniformBufferObject.view = scene->camera->view;
	uniformBufferObject.proj = scene->camera->proj;
	uniformBufferObject.proj[1][1] *= -1;

	void* data;
	vkMapMemory(logicalDevice, uniformBuffer.bufferMemory, 0, sizeof(uniformBufferObject), 0, &data);
	memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
	vkUnmapMemory(logicalDevice, uniformBuffer.bufferMemory);
}

void FVulkanCursor3D::CreateVertexBuffer()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	FMesh* mesh = scene->mesh2;
	VkDeviceSize bufferSize = sizeof(mesh->vertices[0]) * mesh->vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mesh->vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags vertexMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, vertexBufferUsageFlags, vertexMemoryPropertyFlags, vertexBuffer.buffer, vertexBuffer.bufferMemory);

	FVulkanBufferCalculator::CopyBuffer(logicalDevice, commandPool, applicationData->graphicsQueue, stagingBuffer, vertexBuffer.buffer, bufferSize);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void FVulkanCursor3D::CreateIndexBuffer()
{
	auto logicalDevice = applicationData->vulkanDevice.logicalDevice;

	FMesh* mesh = scene->mesh2;
	VkDeviceSize bufferSize = sizeof(mesh->indices[0]) * mesh->indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mesh->indices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	VkBufferUsageFlags indexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VkMemoryPropertyFlags indexMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	FVulkanBufferCalculator::CreateBuffer(applicationData->vulkanDevice, bufferSize, indexBufferUsageFlags, indexMemoryPropertyFlags, indexBuffer.buffer, indexBuffer.bufferMemory);

	FVulkanBufferCalculator::CopyBuffer(logicalDevice, commandPool, applicationData->graphicsQueue, stagingBuffer, indexBuffer.buffer, bufferSize);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}


void FVulkanCursor3D::UpdateCommandBuffers()
{
	auto frameBufferWidth = applicationData->swapChain.extent.width;
	auto frameBufferHeight = applicationData->swapChain.extent.height;

	VkCommandBufferBeginInfo beginInfo = FVulkanInitializers::CommandBufferBeginInfo();
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = FVulkanInitializers::RenderPassBeginInfo();
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent.width = frameBufferWidth;
	renderPassInfo.renderArea.extent.height = frameBufferHeight;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		renderPassInfo.framebuffer = applicationData->swapChain.frameBuffers[i];

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(scene->mesh2->indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void FVulkanCursor3D::Submit(VkQueue queue, uint32_t bufferindex)
{
	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[bufferindex];

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

void FVulkanCursor3D::PrepareRenderPass()
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

VkPipelineInputAssemblyStateCreateInfo* FVulkanCursor3D::CreatePipelineInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo* inputAssembly = new VkPipelineInputAssemblyStateCreateInfo();
	inputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly->primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

VkPipelineColorBlendStateCreateInfo* FVulkanCursor3D::CreatePipelineColorBlendStateCreateInfo()
{
	VkPipelineColorBlendAttachmentState* colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
	colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment->blendEnable = VK_FALSE;
	colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo* colorBlending = new VkPipelineColorBlendStateCreateInfo();
	colorBlending->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending->logicOpEnable = VK_FALSE;
	colorBlending->logicOp = VK_LOGIC_OP_COPY;
	colorBlending->attachmentCount = 1;
	colorBlending->pAttachments = colorBlendAttachment;
	colorBlending->blendConstants[0] = 0.0f;
	colorBlending->blendConstants[1] = 0.0f;
	colorBlending->blendConstants[2] = 0.0f;
	colorBlending->blendConstants[3] = 0.0f;
	return colorBlending;
}

VkPipelineDepthStencilStateCreateInfo* FVulkanCursor3D::CreatePipelineDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo* depthStencil = new VkPipelineDepthStencilStateCreateInfo();
	depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil->depthTestEnable = VK_TRUE;
	depthStencil->depthWriteEnable = VK_TRUE;
	depthStencil->depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil->depthBoundsTestEnable = VK_FALSE;
	depthStencil->minDepthBounds = 0.0f; // Optional
	depthStencil->maxDepthBounds = 1.0f; // Optional
	depthStencil->stencilTestEnable = VK_FALSE;
	depthStencil->front = {};
	depthStencil->back = {};
	return depthStencil;
}
