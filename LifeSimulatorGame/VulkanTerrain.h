#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "MeshVertex.h"
#include <vector>

class FVulkanDevice;
class FScene;
class FTimeManager;
class FGameManager;
struct FTerrainDisplayMesh;

class FVulkanTerrain
{
public:
	void Initialize(FGameManager* gameManager);
	void Destroy(FVulkanDevice vulkanDevice);

public:
	void PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo);
	void CreateBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);

	void UpdateFrame(VkDevice logicalDevice, FScene* scene);

	void BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelinelayout);

	void DestroyBuffers(FVulkanDevice vulkanDevice);

private:
	void UpdateVertexBuffer();

	void *verticesMemory;
	void *indicesMemory;

	VkPipeline graphicsPipeline;
	VkDescriptorSet descriptorSet;

	FTerrainDisplayMesh* terrainDisplayMesh;
	FVulkanBuffer uniformBuffer;
	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;

	FTimeManager* timeManager;

	void CreateUniformBuffer(FVulkanDevice vulkanDevice);;
	void CreateIndexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();

	void CreateVertexBuffer2(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	void UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene);
};

