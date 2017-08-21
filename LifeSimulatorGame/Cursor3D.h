#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include <vector>

class FVulkanDevice;
class FGameManager;
class FScene;
class FVulkanApplication;

class FVulkanCursor3D
{
public:	
	void Initialize(FGameManager* gameManager);
	void Destroy(FVulkanDevice vulkanDevice);
	
	void UpdateFrame(VkDevice logicalDevice, FScene* scene);

	void Submit(VkQueue queue, uint32_t bufferindex);

private:
	void LoadAssets();
	VkCommandPool commandPool;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkRenderPass renderPass;
	// other
	uint32_t *frameBufferWidth;
	uint32_t *frameBufferHeight;
	std::vector<VkFramebuffer*> frameBuffers;

	std::vector<VkCommandBuffer> commandBuffers;

	VkPipeline graphicsPipeline;

	FVulkanBuffer uniformBuffer;
	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;

	void UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene);

	FScene* scene;

	void PrepareResources(FVulkanApplication* application);
	void CreateCommandPool(FVulkanDevice vulkanDevice);
	void CreateCommandBuffer(FVulkanDevice vulkanDevice);
	void UpdateCommandBuffers();
	void CreateBuffers(FVulkanDevice vulkanDevice, VkQueue graphicsQueue);
	void CreateDescriptorPool(FVulkanDevice vulkanDevice);
	void CreateDescriptorSetLayout(VkDevice logicalDevice);
	void CreateDescriptorSet(VkDevice logicalDevice);
	void CreatePipelineCache(FVulkanDevice vulkanDevice);
	void PrepareRenderPass(FVulkanApplication* application);
	void CreateGraphicsPipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo);
	void CreatePipelineLayout(VkDevice logicalDevice);

	void CreateUniformBuffer(FVulkanDevice vulkanDevice);
	void CreateVertexBuffer(FScene* scene, FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateIndexBuffer(FScene* scene, FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();
};

