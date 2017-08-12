#include "VulkanTerrain.h"
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
#include "PerlinNoise.h"

void FVulkanTerrain::LoadAssets(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue)
{
	PerlinNoise Noise;

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;

	for (int x = 0; x < numberOfVertices; x++)
	{
		for (int y = 0; y < numberOfVertices; y++)
		{
			float height = Noise.noise((float) x/ numberOfVertices, float(y)/ numberOfVertices);

			FTerrainVertex Vertex;
			Vertex.pos = { x - numberOfQuads/2, y - numberOfQuads / 2, height };
			vertices.push_back(Vertex);
		}
	}

	for (int x = 0; x < numberOfQuads; x++)
	{
		for (int y = 0; y < numberOfQuads; y++)
		{
			int startIndex = x * numberOfVertices + y;

			indices.push_back(startIndex + 0);
			indices.push_back(startIndex + 0 + numberOfVertices);
			indices.push_back(startIndex + 1);

			indices.push_back(startIndex + 0 + numberOfVertices);
			indices.push_back(startIndex + 1 + numberOfVertices);
			indices.push_back(startIndex + 1);
		}
	}

	//vertices = {
	//	{ { -5, -5, 0.0f } },
	//	{ { -5, 5, 0.0f } },
	//	{ { 5, -5, 0.0f } },
	//	{ { 5, 5, 0.0f } },
	//};
	//indices = { 0, 2, 1, 2, 3, 1 };
}


void FVulkanTerrain::DestroyBuffers(FVulkanDevice vulkanDevice)
{
	uniformBuffer.Destroy(vulkanDevice.logicalDevice);
	indexBuffer.Destroy(vulkanDevice.logicalDevice);
	vertexBuffer.Destroy(vulkanDevice.logicalDevice);

}
void FVulkanTerrain::PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo)
{
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

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
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

void FVulkanTerrain::CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool)
{
	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = FVulkanInitializers::DescriptorSetAllocateInfo();
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
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

	vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void FVulkanTerrain::CreateUniformBuffer(FVulkanDevice vulkanDevice)
{
	VkDeviceSize bufferSize = sizeof(FUniformBufferObject);
	VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, uniformBuffer.buffer, uniformBuffer.bufferMemory);
}

void FVulkanTerrain::UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene)
{
	FUniformBufferObject uniformBufferObject = {};
	uniformBufferObject.model = glm::mat4();

	uniformBufferObject.view = scene->camera->view;
	uniformBufferObject.proj = scene->camera->proj;
	uniformBufferObject.proj[1][1] *= -1;

	void* data;
	vkMapMemory(logicalDevice, uniformBuffer.bufferMemory, 0, sizeof(uniformBufferObject), 0, &data);
	memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
	vkUnmapMemory(logicalDevice, uniformBuffer.bufferMemory);
}

void FVulkanTerrain::BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelineLayout)
{
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void FVulkanTerrain::CreateBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	CreateVertexBuffer(vulkanDevice, commandPool, graphicsQueue);
	CreateIndexBuffer(vulkanDevice, commandPool, graphicsQueue);
	CreateUniformBuffer(vulkanDevice);
}

void FVulkanTerrain::CreateVertexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();


	FVulkanBuffer stagingBuffer;
	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer.buffer, stagingBuffer.bufferMemory);

	void* data;
	vkMapMemory(vulkanDevice.logicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanDevice.logicalDevice, stagingBuffer.bufferMemory);

	VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags vertexMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, vertexBufferUsageFlags, vertexMemoryPropertyFlags, vertexBuffer.buffer, vertexBuffer.bufferMemory);

	FVulkanBufferCalculator::CopyBuffer(vulkanDevice.logicalDevice, commandPool, graphicsQueue, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

	stagingBuffer.Destroy(vulkanDevice.logicalDevice);
}

void FVulkanTerrain::CreateIndexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	FVulkanBuffer stagingBuffer;
	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer.buffer, stagingBuffer.bufferMemory);

	void* data;
	vkMapMemory(vulkanDevice.logicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanDevice.logicalDevice, stagingBuffer.bufferMemory);

	VkBufferUsageFlags indexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VkMemoryPropertyFlags indexMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, indexBufferUsageFlags, indexMemoryPropertyFlags, indexBuffer.buffer, indexBuffer.bufferMemory);

	FVulkanBufferCalculator::CopyBuffer(vulkanDevice.logicalDevice, commandPool, graphicsQueue, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

	stagingBuffer.Destroy(vulkanDevice.logicalDevice);
}

VkPipelineInputAssemblyStateCreateInfo* FVulkanTerrain::CreatePipelineInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo* inputAssembly = new VkPipelineInputAssemblyStateCreateInfo();
	inputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly->primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

VkPipelineColorBlendStateCreateInfo* FVulkanTerrain::CreatePipelineColorBlendStateCreateInfo()
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

VkPipelineDepthStencilStateCreateInfo* FVulkanTerrain::CreatePipelineDepthStencilStateCreateInfo()
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