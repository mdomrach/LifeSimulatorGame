#pragma once

#include <vulkan/vulkan.h>
#include "Fonts/stb_font_consolas_24_latin1.inl"
#include <vector>
#include <glm/glm.hpp>
#include "VulkanBuffer.h"
#include "TextOverlay.h"

// Defines for the STB font used
// STB font files can be found at http://nothings.org/stb/font/
#define STB_FONT_NAME stb_font_consolas_24_latin1
#define STB_FONT_WIDTH STB_FONT_consolas_24_latin1_BITMAP_WIDTH
#define STB_FONT_HEIGHT STB_FONT_consolas_24_latin1_BITMAP_HEIGHT 
#define STB_FIRST_CHAR STB_FONT_consolas_24_latin1_FIRST_CHAR
#define STB_NUM_CHARS STB_FONT_consolas_24_latin1_NUM_CHARS

// Max. number of chars the text overlay buffer can hold
#define TEXTOVERLAY_MAX_CHAR_COUNT 2048

class FVulkanApplication;
class FVulkanDevice;
class FTimeManager;
class FVulkanApplicationData;
class FGameManager;

class FVulkanTextOverlay
{
public:
	void Initialize(FGameManager* gameManager);
	void UpdateSwapChain();
	void Destroy();

	void Submit(VkQueue queue, uint32_t bufferindex);

	void UpdateTextOverlay();
private:
	FVulkanApplicationData* applicationData;
	FTextOverlay* textOverlay;


	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline graphicsPipeline;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkSampler sampler;
	VkImage image;
	VkImageView imageView;
	FVulkanBuffer vertexBuffer;
	VkDeviceMemory imageMemory;

	std::vector<VkCommandBuffer> commandBuffers;

	// Pointer to mapped vertex buffer
	glm::vec4 *mapped = nullptr;

	uint32_t numLetters;
	stb_fontchar stbFontData[STB_NUM_CHARS];
	
	void CreateCommandPool();
	void CreateCommandBuffer();
	void UpdateCommandBuffers();
	void CreateVertexBuffer();
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

	void BeginTextUpdate(VkDevice logicalDevice);
	void AddText(std::string text, float x, float y, ETextAlign align);
	void EndTextUpdate(VkDevice logicalDevice);
};


