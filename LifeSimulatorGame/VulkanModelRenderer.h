#pragma once

#include <vulkan/vulkan.h>
#include "Environment.h"
//#include "ParticleFire.h"
#include "VulkanTerrain.h"

class FVulkanDevice;
class FGameManager;
class FScene;
class FVulkanApplication;

class FVulkanModelRenderer
{
public:
	void Initialize(FGameManager* gameManager);
	void InitializeVulkan(FVulkanDevice vulkanDevice);
	void UpdateSwapChain(FVulkanDevice vulkanDevice);

	//void Destroy(FVulkanDevice vulkanDevice);
	void Destroy(FVulkanDevice vulkanDevice);
	//void DestroyBuffers(FVulkanDevice vulkanDevice);
	void DestroyPipelines();
	void FreeCommandBuffers();

	void UpdateFrame(VkDevice logicalDevice);

	void Submit(VkQueue graphicsQueue, uint32_t bufferindex);

	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();

public:
	void CreateBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void CreateDescriptorPool(FVulkanDevice vulkanDevice);
	void CreateDescriptorSets(FVulkanDevice vulkanDevice);
	void CreateCommandBuffers(FVulkanDevice vulkanDevice);
	void BuildCommandBuffers(FVulkanDevice vulkanDevice);
	//void CreateSemaphores(FVulkanDevice vulkanDevice);

private:
	FEnvironment environment;
	//FParticleFire particleFire;
	FVulkanTerrain terrain;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkCommandBuffer> commandBuffers;

	FVulkanApplication* vulkanApplication;
	FScene* scene;
	VkRenderPass renderPass;

//private:
//	void CreateCommandPool(FVulkanDevice vulkanDevice);
//	void CreateCommandBuffer(FVulkanDevice vulkanDevice);
//	void CreateBuffers(FVulkanDevice vulkanDevice);
//	void CreateDescriptorPool(FVulkanDevice vulkanDevice);
//	void CreateDescriptorSetLayout(FVulkanDevice vulkanDevice);
//	void CreatePipelineLayout(FVulkanDevice vulkanDevice);
//	void CreateDescriptorSet(FVulkanDevice vulkanDevice);
//	void CreatePipelineCache(FVulkanDevice vulkanDevice);


};

