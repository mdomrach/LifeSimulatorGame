#include "ParticleFire.h"
#include "FileCalculator.h"
#include "ShaderCalculator.h"
#include "VulkanInitializers.h"
#include "Particle.h"
#include "VulkanDevice.h"
#include "VulkanBufferCalculator.h"
#include "UniformBufferObjectParticle.h"
#include "TimeManager.h"
#include "Scene.h"
#include "Camera.h"
#include "VulkanApplication.h"
#include "Random.h"
#include "VulkanTextureCreateInfo.h"
#include "GameManager.h"

void FParticleFire::Initialize(FGameManager* gameManager)
{
	timeManager = gameManager->timeManager;
}

void FParticleFire::LoadAssets(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue)
{
	FVulkanTextureCreateInfo smokeTextureCreateInfo = {};
	smokeTextureCreateInfo.filename = SMOKE_TEXTURE_PATH;
	smokeTextureCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	smokeTextureCreateInfo.Create(vulkanDevice, commandPool, queue, smokeTexture);
	
	FVulkanTextureCreateInfo fireTextureCreateInfo = {};
	fireTextureCreateInfo.filename = FIRE_TEXTURE_PATH;
	fireTextureCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	fireTextureCreateInfo.Create(vulkanDevice, commandPool, queue, fireTexture);


	// Create a custom sampler to be used with the particle textures
	// Create sampler
	VkSamplerCreateInfo samplerCreateInfo = FVulkanInitializers::SamplerCreateInfo();
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	// Different address mode
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
	samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	// Both particle textures have the same number of mip maps
	samplerCreateInfo.maxLod = 0.0f;
	//samplerCreateInfo.maxLod = float(textures.particles.fire.mipLevels);
	// Enable anisotropic filtering
	samplerCreateInfo.maxAnisotropy = 8.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	// Use a different border color (than the normal texture loader) for additive blending
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	if (vkCreateSampler(vulkanDevice.logicalDevice, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create custom sampler for particles!");
	}
}

void FParticleFire::Destroy(FVulkanDevice vulkanDevice)
{
	smokeTexture.Destroy(vulkanDevice);
	fireTexture.Destroy(vulkanDevice);

	vkDestroySampler(vulkanDevice.logicalDevice, sampler, nullptr);
	// need to do this at cleanupswapchain
	//vkDestroyPipeline(vulkanDevice.logicalDevice, graphicsPipeline, nullptr);
}

void FParticleFire::DestroyBuffers(FVulkanDevice vulkanDevice)
{
	vkUnmapMemory(vulkanDevice.logicalDevice, particleBuffer.bufferMemory);
	uniformBuffer.Destroy(vulkanDevice.logicalDevice);
	particleBuffer.Destroy(vulkanDevice.logicalDevice);
}

void FParticleFire::PreparePipeline(VkDevice logicalDevice, VkGraphicsPipelineCreateInfo* pipelineInfo)
{
	pipelineInfo->pInputAssemblyState = CreatePipelineInputAssemblyStateCreateInfo();
	pipelineInfo->pDepthStencilState = CreatePipelineDepthStencilStateCreateInfo();
	pipelineInfo->pColorBlendState = CreatePipelineColorBlendStateCreateInfo();

	auto vertShaderCode = FFileCalculator::ReadFile("shaders/particle.vert.spv");
	auto fragShaderCode = FFileCalculator::ReadFile("shaders/particle.frag.spv");

	VkShaderModule vertShaderModule = FShaderCalculator::CreateShaderModule(logicalDevice, vertShaderCode);
	VkShaderModule fragShaderModule = FShaderCalculator::CreateShaderModule(logicalDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";


	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
	
	auto vertexBindingDescription = FParticle::GetVertexBindingDescription();
	auto vertexAttributeDescriptions = FParticle::GetVertexAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = FVulkanInitializers::PipelineVertexInputStateCreateInfo();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	pipelineInfo->stageCount = 2;
	pipelineInfo->pStages = shaderStages.data();
	pipelineInfo->pVertexInputState = &vertexInputInfo;

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);

	delete pipelineInfo->pInputAssemblyState;
	delete pipelineInfo->pDepthStencilState;
	delete pipelineInfo->pColorBlendState->pAttachments;
	delete pipelineInfo->pColorBlendState;
}

void FParticleFire::CreateDescriptorSets(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool)
{
	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = FVulkanInitializers::DescriptorSetAllocateInfo();
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(FUniformBufferObjectParticle);

	// Image descriptor for the color map texture
	VkDescriptorImageInfo texDescriptorSmoke{};
	texDescriptorSmoke.sampler = sampler;
	texDescriptorSmoke.imageView = smokeTexture.imageView;
	texDescriptorSmoke.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkDescriptorImageInfo texDescriptorFire{};
	texDescriptorFire.sampler = sampler;
	texDescriptorFire.imageView = fireTexture.imageView;
	texDescriptorFire.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	
	std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
	descriptorWrites[0] = FVulkanInitializers::WriteDescriptorSet();
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1] = FVulkanInitializers::WriteDescriptorSet();
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &texDescriptorSmoke;

	descriptorWrites[2] = FVulkanInitializers::WriteDescriptorSet();
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &texDescriptorFire;

	vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void FParticleFire::CreateBuffers(FVulkanDevice vulkanDevice)
{
	CreateUniformBuffer(vulkanDevice);
	CreateParticleBuffer(vulkanDevice);
}

void FParticleFire::CreateUniformBuffer(FVulkanDevice vulkanDevice)
{
	VkDeviceSize bufferSize = sizeof(FUniformBufferObjectParticle);
	VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, uniformBuffer.buffer, uniformBuffer.bufferMemory);
}

void FParticleFire::UpdateUniformBuffer(VkDevice logicalDevice, FScene* scene)
{
	FUniformBufferObjectParticle uniformBufferObject = {};
	uniformBufferObject.model = glm::mat4();
	
	//uniformBufferObject.view = scene->camera->view;
	uniformBufferObject.projection = scene->camera->proj;
	//uniformBufferObject.projection[1][1] *= -1;

	//uniformBufferObject.viewportDim = glm::vec2((float)FVulkanApplication::WIDTH, (float)FVulkanApplication::HEIGHT);
	uniformBufferObject.viewportDim = glm::vec2(800, 600);

	void* data;
	vkMapMemory(logicalDevice, uniformBuffer.bufferMemory, 0, sizeof(uniformBufferObject), 0, &data);
	memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
	vkUnmapMemory(logicalDevice, uniformBuffer.bufferMemory);
}

void FParticleFire::BuildCommandBuffers(VkCommandBuffer commandBuffer, FScene* scene, VkPipelineLayout pipelineLayout)
{
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdBindVertexBuffers(commandBuffer, VERTEX_BUFFER_BIND_ID, 1, &particleBuffer.buffer, offsets);
	vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);
}


void FParticleFire::CreateParticleBuffer(FVulkanDevice vulkanDevice)
{
	particles.resize(PARTICLE_COUNT);
	for (auto& particle : particles)
	{
		InitParticle(&particle, emitterPos);
		particle.alpha = 1.0f - (abs(particle.pos.y) / (FLAME_RADIUS * 2.0f));
	}

	VkDeviceSize bufferSize = sizeof(FParticle) * particles.size();

	VkBufferUsageFlags particleBufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags particleMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, bufferSize, particleBufferUsageFlags, particleMemoryPropertyFlags, particleBuffer.buffer, particleBuffer.bufferMemory);

	if (vkMapMemory(vulkanDevice.logicalDevice, particleBuffer.bufferMemory, 0, bufferSize, 0, &mappedMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map particle memory!");		
	}
}

void FParticleFire::InitParticle(FParticle *particle, glm::vec3 emitterPos)
{
	particle->vel = glm::vec4(0.0f, minVel.y + FRandom::Range(maxVel.y - minVel.y), 0.0f, 0.0f);
	particle->alpha = FRandom::Range(0.75f);
	particle->size = 1.0f + FRandom::Range(0.5f);
	particle->color = glm::vec4(1.0f);
	particle->type = PARTICLE_TYPE_FLAME;
	particle->rotation = FRandom::Range(2.0f * float(M_PI));
	particle->rotationSpeed = FRandom::Range(2.0f) - FRandom::Range(2.0f);

	// Get random sphere point
	float theta = FRandom::Range(2.0f * float(M_PI));
	float phi = FRandom::Range(float(M_PI)) - float(M_PI) / 2.0f;
	float r = FRandom::Range(FLAME_RADIUS);

	particle->pos.x = r * cos(theta) * cos(phi);
	particle->pos.y = r * sin(phi);
	particle->pos.z = r * sin(theta) * cos(phi);

	particle->pos += glm::vec4(emitterPos, 0.0f);
}


void FParticleFire::TransitionParticle(FParticle *particle)
{
	switch (particle->type)
	{
	case PARTICLE_TYPE_FLAME:
		// Flame particles have a chance of turning into smoke
		if (FRandom::Range(1.0f) < 0.05f)
		{
			particle->alpha = 0.0f;
			particle->color = glm::vec4(0.25f + FRandom::Range(0.25f));
			particle->pos.x *= 0.5f;
			particle->pos.z *= 0.5f;
			particle->vel = glm::vec4(FRandom::Range(1.0f) - FRandom::Range(1.0f), (minVel.y * 2) + FRandom::Range(maxVel.y - minVel.y), FRandom::Range(1.0f) - FRandom::Range(1.0f), 0.0f);
			particle->size = 1.0f + FRandom::Range(0.5f);
			particle->rotationSpeed = FRandom::Range(1.0f) - FRandom::Range(1.0f);
			particle->type = PARTICLE_TYPE_SMOKE;
		}
		else
		{
			InitParticle(particle, emitterPos);
		}
		break;
	case PARTICLE_TYPE_SMOKE:
		// Respawn at end of life
		InitParticle(particle, emitterPos);
		break;
	}
}

void FParticleFire::UpdateParticles()
{
	float particleTimer = timeManager->deltaFrameTime * 0.45f;
	for (auto& particle : particles)
	{
		switch (particle.type)
		{
		case PARTICLE_TYPE_FLAME:
			particle.pos.y -= particle.vel.y * particleTimer * 3.5f;
			particle.alpha += particleTimer * 2.5f;
			particle.size -= particleTimer * 0.5f;
			break;
		case PARTICLE_TYPE_SMOKE:
			particle.pos -= particle.vel * timeManager->deltaFrameTime * 1.0f;
			particle.alpha += particleTimer * 1.25f;
			particle.size += particleTimer * 0.125f;
			particle.color -= particleTimer * 0.05f;
			break;
		}
		particle.rotation += particleTimer * particle.rotationSpeed;
		// Transition particle state
		if (particle.alpha > 2.0f)
		{
			TransitionParticle(&particle);
		}
	}
	size_t size = sizeof(FParticle) * particles.size();
	memcpy(mappedMemory, particles.data(), size);
}

VkPipelineInputAssemblyStateCreateInfo* FParticleFire::CreatePipelineInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo* inputAssembly = new VkPipelineInputAssemblyStateCreateInfo();
	inputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	inputAssembly->primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

VkPipelineColorBlendStateCreateInfo* FParticleFire::CreatePipelineColorBlendStateCreateInfo()
{
	VkPipelineColorBlendAttachmentState* colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
	colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment->blendEnable = VK_TRUE;
	colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo* colorBlending = new VkPipelineColorBlendStateCreateInfo();
	colorBlending->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending->logicOpEnable = VK_FALSE;
	colorBlending->logicOp = VK_LOGIC_OP_COPY;
	colorBlending->attachmentCount = 1;
	colorBlending->pAttachments = colorBlendAttachment;
	colorBlending->blendConstants[0] = 0.0f;
	colorBlending->blendConstants[1] = 0.0f;
	colorBlending->blendConstants[2] = 0.0f;
	colorBlending->blendConstants[3] = 0.0f;
	return colorBlending;
}

VkPipelineDepthStencilStateCreateInfo* FParticleFire::CreatePipelineDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo* depthStencil = new VkPipelineDepthStencilStateCreateInfo();
	depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil->depthTestEnable = VK_TRUE;
	depthStencil->depthWriteEnable = VK_FALSE;
	depthStencil->depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil->depthBoundsTestEnable = VK_FALSE;
	depthStencil->minDepthBounds = 0.0f; // Optional
	depthStencil->maxDepthBounds = 1.0f; // Optional
	depthStencil->stencilTestEnable = VK_FALSE;
	depthStencil->front = {};
	depthStencil->back = {};
	return depthStencil;
}