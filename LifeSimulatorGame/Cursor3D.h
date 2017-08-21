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
class FVulkanApplicationData;

class FVulkanCursor3D
{
public:
	void Initialize(FGameManager* gameManager);
	void UpdateSwapChain();
	void Destroy();
	
	void UpdateFrame();

	void Submit(VkQueue queue, uint32_t bufferindex);

private:
	FVulkanApplicationData* applicationData;
	FScene* scene;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline graphicsPipeline;
	VkRenderPass renderPass;
	VkCommandPool commandPool;

	FVulkanBuffer uniformBuffer;
	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;

	std::vector<VkCommandBuffer> commandBuffers;

	void LoadAssets();

	void UpdateUniformBuffer();


	void CreateCommandPool();
	void CreateCommandBuffer();
	void UpdateCommandBuffers();
	void CreateBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSet();
	void CreatePipelineCache();
	void PrepareRenderPass();
	void CreateGraphicsPipeline();
	void CreatePipelineLayout();

	void CreateUniformBuffer();
	void CreateVertexBuffer();
	void CreateIndexBuffer();

	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();
};

