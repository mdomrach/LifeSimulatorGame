#pragma once

//#include <vulkan/vulkan.h>
//#include <string>
//#include "VulkanBuffer.h"
//#include "VulkanTexture.h"
//#include <vector>
//
//class FVulkanDevice;
//class FGameManager;
//class FScene;
//class FVulkanApplication;
//class FVulkanApplicationData;

//#include <vulkan/vulkan.h>
//#include "Fonts/stb_font_consolas_24_latin1.inl"
//#include <vector>
//#include <glm/glm.hpp>
//#include "VulkanBuffer.h"
//#include "TextOverlay.h"
#include "VulkanTextOverlay.h"
#include "TerrainVertex.h"

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

	FVulkanBuffer uniformBuffer;
	FVulkanBuffer vertexBuffer;
	FVulkanBuffer indexBuffer;
	VkPipeline graphicsPipeline;

private:

	FVulkanApplicationData* applicationData;
	FTextOverlay* textOverlay;
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
	//glm::vec4 *mapped = nullptr;
	struct FTerrainVertex* mapped = nullptr;

	uint32_t numLetters;
	stb_fontchar stbFontData[STB_NUM_CHARS];

	void CreateCommandPool();
	void CreateCommandBuffer();
	void UpdateCommandBuffers();
	void CreateUniformBuffer();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
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

