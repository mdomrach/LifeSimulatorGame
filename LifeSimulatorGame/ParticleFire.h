#pragma once

#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include <vector>

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Particle.h"
//#include <glm/gtc/matrix_transform.hpp>

class FScene;
class FVulkanDevice;
class FTimeManager;
class FGameManager;

#define PARTICLE_COUNT 512
#define FLAME_RADIUS 8.0f
#define M_PI 3.1415

class FParticleFire
{
public:
	VkDescriptorSet descriptorSet;
	VkPipeline graphicsPipeline;
	void *mappedMemory;
	size_t size;
	FVulkanBuffer uniformBuffer;
	FVulkanBuffer particleBuffer;

	std::vector<FParticle> particles;

	FVulkanTexture smokeTexture;
	FVulkanTexture fireTexture;
	// Use a custom sampler to change sampler attributes required for rotating the uvs in the shader for alpha blended textures
	VkSampler sampler;
	glm::vec3 rotation = glm::vec3();
	
	void Initialize(FGameManager* gameManager);
	void LoadAssets(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue);
	void Destroy(FVulkanDevice vulkanDevice);
	void DestroyBuffers(FVulkanDevice vulkanDevice);
	void PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo);
	void CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
	void CreateBuffers(FVulkanDevice vulkanDevice);
	void UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene);
	void BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelinelayout);

	void UpdateParticles();
private:
	const std::string SMOKE_TEXTURE_PATH = "Textures/particle_smoke.jpg";
	const std::string FIRE_TEXTURE_PATH = "Textures/particle_fire.jpg";

	glm::vec3 emitterPos = glm::vec3(0.0f, -FLAME_RADIUS + 2.0f, 0.0f);
	glm::vec3 minVel = glm::vec3(-3.0f, 0.5f, -3.0f);
	glm::vec3 maxVel = glm::vec3(3.0f, 7.0f, 3.0f);

	VkPipelineInputAssemblyStateCreateInfo* CreatePipelineInputAssemblyStateCreateInfo();
	VkPipelineColorBlendStateCreateInfo* CreatePipelineColorBlendStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo* CreatePipelineDepthStencilStateCreateInfo();

	void CreateUniformBuffer(FVulkanDevice vulkanDevice);
	void CreateParticleBuffer(FVulkanDevice vulkanDevice);
	void InitParticle(FParticle *particle, glm::vec3 emitterPos);
	void TransitionParticle(FParticle *particle);

	FTimeManager* timeManager;
};

