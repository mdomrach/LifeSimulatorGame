#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "TerrainVertex.h"
#include <vector>

class FVulkanDevice;
class FScene;
class FTimeManager;
class FGameManager;
class FTerrain;

class FVulkanTerrain
{
public:	
	void Initialize(FGameManager* gameManager);

	void LoadAssets();

	void PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo);
	void CreateBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);

	void UpdateFrame(VkDevice logicalDevice, FScene* scene);

	void BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelinelayout);

	void DestroyBuffers(FVulkanDevice vulkanDevice);

private:
	void UpdateVertexBuffer();
	void UpdateNormals();

	void *verticesMemory;
	void *indicesMemory;

	VkPipeline graphicsPipeline;
	VkDescriptorSet descriptorSet;

	FTerrain* terrain;
	FVulkanBuffer uniformBuffer;
	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;

	FTimeManager* timeManager;

	void CreateUniformBuffer(FVulkanDevice vulkanDevice);
	void CreateVertexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateIndexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;
	int GetVertexIndex(int x, int y);

	void CreateVertexBuffer2(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateIndexBuffer2(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	void UpdateIndexBuffer();
	void UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene);
};

