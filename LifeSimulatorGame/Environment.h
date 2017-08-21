#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

class FVulkanDevice;
class FScene;
class FGameManager;

class FEnvironment
{
public:
	void Initialize(FGameManager* gameManager);
	void Destroy(FVulkanDevice vulkanDevice);
	void UpdateFrame();
	void Submit(VkQueue graphicsQueue, uint32_t bufferindex);

public:	
	VkPipeline graphicsPipeline;
	VkDescriptorSet descriptorSet;
	FVulkanBuffer uniformBuffer;

	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;

	void DestroyBuffers(FVulkanDevice vulkanDevice);

	void PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo);
	//void LoadAssets(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue);
	void CreateBuffers(FScene* scene, FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);

	void UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene);

	void BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelinelayout);

private:
	void CreateUniformBuffer(FVulkanDevice vulkanDevice);
	void CreateVertexBuffer(FScene* scene, FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateIndexBuffer(FScene* scene, FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
		
	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();
};

