#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "TerrainVertex.h"
#include <vector>

class FVulkanDevice;
class FScene;

class FVulkanTerrain
{
public:
	std::vector<FTerrainVertex> vertices;
	std::vector<uint32_t> indices;

	FVulkanTexture texture;

	VkPipeline graphicsPipeline;
	VkDescriptorSet descriptorSet;

	FVulkanBuffer uniformBuffer;
	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;

	void LoadAssets(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue);
	void DestroyBuffers(FVulkanDevice vulkanDevice);

	void PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo);
	void CreateBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);

	void UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene);

	void BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelinelayout);

private:
	void CreateUniformBuffer(FVulkanDevice vulkanDevice);
	void CreateVertexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateIndexBuffer(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;
	int GetVertexIndex(int x, int y);
};

