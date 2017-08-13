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
#include "GameManager.h"
#include "TimeManager.h"

void FVulkanTerrain::Initialize(FGameManager* gameManager)
{
	timeManager = gameManager->timeManager;
}

void FVulkanTerrain::LoadAssets()
{
	PerlinNoise Noise;

	const float noiseAmplitude = 1.2f;
	const float noiseFrequency = 2.5f / numberOfVertices;

	for (int x = 0; x < numberOfVertices; x++)
	{
		for (int y = 0; y < numberOfVertices; y++)
		{
			float height = noiseAmplitude * Noise.noise( x * noiseFrequency, y * noiseFrequency);

			FTerrainVertex Vertex;
			Vertex.pos = { x - numberOfQuads/2, y - numberOfQuads / 2, height };
			Vertex.normal = glm::vec3(0, 0, 1);
			vertices.push_back(Vertex);
		}
	}
	
	for (int x = 1; x < numberOfVertices - 1; x++)
	{
		for (int y = 1; y < numberOfVertices - 1; y++)
		{
			int index11 = GetVertexIndex(x + 0, y + 0);
			glm::vec3 height11 = vertices[index11].pos;

			int index01 = GetVertexIndex(x - 1, y + 0);
			int index10 = GetVertexIndex(x + 0, y - 1);
			int index21 = GetVertexIndex(x + 1, y + 0);
			int index12 = GetVertexIndex(x + 0, y + 1);

			glm::vec3 height01 = vertices[index01].pos;
			glm::vec3 height21 = vertices[index21].pos;
			glm::vec3 height10 = vertices[index10].pos;
			glm::vec3 height12 = vertices[index12].pos;

			glm::vec3 normal1 = glm::cross(height01 - height11, height10 - height11);
			glm::vec3 normal2 = glm::cross(height21 - height11, height12 - height11);
			vertices[index11].normal = normal1 + normal2;
		}
	}

	for (int x = 0; x < numberOfQuads; x++)
	{
		for (int y = 0; y < numberOfQuads; y++)
		{
			int index00 = GetVertexIndex(x + 0, y + 0);
			int index01 = GetVertexIndex(x + 0, y + 1);
			int index10 = GetVertexIndex(x + 1, y + 0);
			int index11 = GetVertexIndex(x + 1, y + 1);

			indices.push_back(index00);
			indices.push_back(index10);
			indices.push_back(index01);

			indices.push_back(index10);
			indices.push_back(index11);
			indices.push_back(index01);
		}
	}
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

void FVulkanTerrain::UpdateFrame(VkDevice logicalDevice, FScene* scene)
{
	UpdateUniformBuffer(logicalDevice, scene);
	UpdateVertexBuffer();
	//UpdateIndexBuffer();
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
	CreateVertexBuffer2(vulkanDevice, commandPool, graphicsQueue);
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

void FVulkanTerrain::CreateVertexBuffer2(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, vertexBuffer.buffer, vertexBuffer.bufferMemory);
	
	if (vkMapMemory(vulkanDevice.logicalDevice, vertexBuffer.bufferMemory, 0, bufferSize, 0, &verticesMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map particle memory!");
	}
}

void FVulkanTerrain::CreateIndexBuffer2(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, indexBuffer.buffer, indexBuffer.bufferMemory);

	if (vkMapMemory(vulkanDevice.logicalDevice, indexBuffer.bufferMemory, 0, bufferSize, 0, &indicesMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map particle memory!");
	}
}

void FVulkanTerrain::UpdateVertexBuffer()
{
	PerlinNoise Noise;

	const float noiseAmplitude = 1.2f;
	const float noiseFrequency = 2.5f / numberOfVertices;
	float offset = timeManager->startFrameTime;

	int i = 0;
	for (int x = 0; x < numberOfVertices; x++)
	{
		for (int y = 0; y < numberOfVertices; y++)
		{
			float height = noiseAmplitude * Noise.noise((x+ offset) * noiseFrequency, y * noiseFrequency);

			vertices[i].pos = { x - numberOfQuads / 2, y - numberOfQuads / 2, height };
			vertices[i].normal = glm::vec3(0, 0, 1);
			i++;
		}
	}

	for (int x = 1; x < numberOfVertices - 1; x++)
	{
		for (int y = 1; y < numberOfVertices - 1; y++)
		{
			int index11 = GetVertexIndex(x + 0, y + 0);
			glm::vec3 height11 = vertices[index11].pos;

			int index01 = GetVertexIndex(x - 1, y + 0);
			int index10 = GetVertexIndex(x + 0, y - 1);
			int index21 = GetVertexIndex(x + 1, y + 0);
			int index12 = GetVertexIndex(x + 0, y + 1);

			glm::vec3 height01 = vertices[index01].pos;
			glm::vec3 height21 = vertices[index21].pos;
			glm::vec3 height10 = vertices[index10].pos;
			glm::vec3 height12 = vertices[index12].pos;

			glm::vec3 normal1 = glm::cross(height01 - height11, height10 - height11);
			glm::vec3 normal2 = glm::cross(height21 - height11, height12 - height11);
			vertices[index11].normal = normal1 + normal2;
		}
	}

	size_t size = sizeof(vertices[0]) * vertices.size();
	memcpy(verticesMemory, vertices.data(), size);
}

void FVulkanTerrain::UpdateIndexBuffer()
{
	int i = 0;
	for (int x = 0; x < numberOfQuads; x++)
	{
		for (int y = 0; y < numberOfQuads; y++)
		{
			int index00 = GetVertexIndex(x + 0, y + 0);
			int index01 = GetVertexIndex(x + 0, y + 1);
			int index10 = GetVertexIndex(x + 1, y + 0);
			int index11 = GetVertexIndex(x + 1, y + 1);

			indices[i++] = index00;
			indices[i++] = index10;
			indices[i++] = index01;

			indices[i++] = index10;
			indices[i++] = index11;
			indices[i++] = index01;
		}
	}

	size_t size = sizeof(indices[0]) * indices.size();
	memcpy(indicesMemory, indices.data(), size);
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

int FVulkanTerrain::GetVertexIndex(int x, int y)
{
	return x * numberOfVertices + y;
}