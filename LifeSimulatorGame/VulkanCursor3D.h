#pragma once

#include "VulkanTextOverlay.h"
#include "MeshVertex.h"

class FVulkanApplication;
class FVulkanDevice;
class FTimeManager;
class FVulkanApplicationData;
class FGameManager;
class FScene;

class FVulkanCursor3D
{

public:
	void Initialize(FGameManager* gameManager);
	void UpdateSwapChain();
	void Destroy();

	void Submit(VkQueue queue, uint32_t bufferindex);
	void UpdateFrame();

	std::vector<VkCommandBuffer> commandBuffers;
	
	size_t dynamicAlignment;
	FVulkanBuffer uniformBuffer;
	std::vector<FVulkanBuffer> vertexBuffers;
	std::vector<FVulkanBuffer> indexBuffers;
	VkPipeline graphicsPipeline;

private:

	FVulkanApplicationData* applicationData;
	FScene* scene;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	// Pointer to mapped vertex buffer
	struct FMeshVertex* mapped = nullptr;

	uint32_t numLetters;
	stb_fontchar stbFontData[STB_NUM_CHARS];

	void CreateCommandPool();
	void CreateCommandBuffer();
	void UpdateCommandBuffers();
	void CreateUniformBuffer();
	void CreateVertexBuffers();
	void CreateIndexBuffers();
	void CreateFontTexture();
	void CreateDescriptorPool();
	void CreateDescriptorSetLayout();
	void CreatePipelineLayout();
	void CreateDescriptorSet();
	void CreatePipelineCache();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();
};

