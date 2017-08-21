#pragma once

#include <vulkan/vulkan.h>
#include "Fonts/stb_font_consolas_24_latin1.inl"
#include <vector>
#include <glm/glm.hpp>
#include "VulkanBuffer.h"

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

class FTextOverlay
{
public:
	enum ETextAlign { alignLeft, alignCenter, alignRight };

	void UpdateSwapChain(FVulkanApplication* vulkanApplication, FVulkanDevice vulkanDevice);
	void Destroy(FVulkanDevice* vulkanDevice);

	void UpdateFrame(FVulkanDevice vulkanDevice);

	void Submit(VkQueue queue, uint32_t bufferindex);

private:

	//references
	VkQueue queue;


	// self
	VkSampler sampler;
	VkImage image;
	VkImageView imageView;
	FVulkanBuffer vertexBuffer;
	VkDeviceMemory imageMemory;

	// general
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	bool visible = true;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	// reference but empty?
	std::vector<VkCommandBuffer> cmdBuffers;

	// other
	uint32_t *frameBufferWidth;
	uint32_t *frameBufferHeight;
	std::vector<VkFramebuffer*> frameBuffers;

	// Pointer to mapped vertex buffer
	glm::vec4 *mapped = nullptr;

	uint32_t numLetters;
	stb_fontchar stbFontData[STB_NUM_CHARS];
	void InitializeHelper(FVulkanApplication* application, std::vector<VkPipelineShaderStageCreateInfo> shaderStages);

	void PrepareResources(FVulkanApplication* application);
	void PrepareRenderPass(FVulkanApplication* application);
	void PreparePipeline(FVulkanApplication* application);

	void BeginTextUpdate(FVulkanDevice vulkanDevice);
	void AddText(std::string text, float x, float y, ETextAlign align);
	void EndTextUpdate(FVulkanDevice vulkanDevice);
	void UpdateCommandBuffers();

	void UpdateTextOverlay(FVulkanDevice vulkanDevice);
	int frameCount;
	float nextFPSUpdateTime;

	FTimeManager* timeManager;


	void CreateCommandPool(FVulkanDevice vulkanDevice);
	void CreateCommandBuffer(FVulkanDevice vulkanDevice);
	void CreateVertexBuffer(FVulkanDevice vulkanDevice);
	void CreateFontTexture(FVulkanDevice vulkanDevice, VkQueue graphicsQueue);
	void CreateDescriptorPool(FVulkanDevice vulkanDevice);
	void CreateDescriptorSetLayout(FVulkanDevice vulkanDevice);
	void CreatePipelineLayout(FVulkanDevice vulkanDevice);
	void CreateDescriptorSet(FVulkanDevice vulkanDevice);
	void CreatePipelineCache(FVulkanDevice vulkanDevice);
};


