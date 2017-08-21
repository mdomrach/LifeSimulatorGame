#include "VulkanModelRenderer.h"
#include "VulkanDevice.h"
#include "GameManager.h"
#include "VulkanApplication.h"
#include "VulkanInitializers.h"
#include "VulkanPipelineCalculator.h"
#include "VulkanCalculator.h"

void FVulkanModelRenderer::Initialize(FGameManager* gameManager)
{
	vulkanApplication = gameManager->vulkanApplication;
	scene = gameManager->scene;

	environment.Initialize(gameManager);
	terrain.Initialize(gameManager);
	terrain.LoadAssets();
}

void FVulkanModelRenderer::InitializeVulkan(FVulkanDevice vulkanDevice)
{
	CreateDescriptorSetLayout();
	CreateBuffers(vulkanDevice, vulkanApplication->commandPool, vulkanApplication->graphicsQueue);
	CreateDescriptorPool(vulkanDevice);
	CreateDescriptorSets(vulkanDevice);
}

void FVulkanModelRenderer::UpdateSwapChain(FVulkanDevice vulkanDevice)
{
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateCommandBuffers(vulkanDevice);
	BuildCommandBuffers(vulkanDevice);
}

void FVulkanModelRenderer::CreateGraphicsPipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = FVulkanInitializers::PipelineLayoutCreateInfo();
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(vulkanApplication->vulkanDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &vulkanApplication->pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo* pipelineInfo = FVulkanPipelineCalculator::CreateGraphicsPipelineInfo(vulkanApplication->swapChain, descriptorSetLayout, vulkanApplication->vulkanDevice.logicalDevice, renderPass, vulkanApplication->pipelineLayout);

	environment.PreparePipeline(vulkanApplication->vulkanDevice.logicalDevice, pipelineInfo);
	//particleFire.PreparePipeline(vulkanDevice.logicalDevice, pipelineInfo);
	terrain.PreparePipeline(vulkanApplication->vulkanDevice.logicalDevice, pipelineInfo);

	FVulkanPipelineCalculator::DeleteGraphicsPipelineInfo(pipelineInfo);
}

void FVulkanModelRenderer::UpdateFrame(VkDevice logicalDevice)
{
	environment.UpdateUniformBuffer(logicalDevice, scene);
	//particleFire.UpdateUniformBuffer(vulkanDevice.logicalDevice, scene);
	//particleFire.UpdateParticles();
	terrain.UpdateFrame(logicalDevice, scene);
}

void FVulkanModelRenderer::Destroy(FVulkanDevice vulkanDevice)
{
	environment.Destroy(vulkanDevice);
	//particleFire.Destroy(vulkanDevice);

	vkDestroyDescriptorPool(vulkanDevice.logicalDevice, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(vulkanDevice.logicalDevice, descriptorSetLayout, nullptr);
//}
//
//void FVulkanModelRenderer::DestroyBuffers(FVulkanDevice vulkanDevice)
//{
	environment.DestroyBuffers(vulkanDevice);
	//particleFire.DestroyBuffers(vulkanDevice);
	terrain.DestroyBuffers(vulkanDevice);
}

void FVulkanModelRenderer::Submit(VkQueue graphicsQueue, uint32_t bufferindex)
{
	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();

	VkSemaphore waitSemaphores[] = { vulkanApplication->imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[bufferindex];

	VkSemaphore signalSemaphores[] = { vulkanApplication->renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void FVulkanModelRenderer::DestroyPipelines()
{
	vkDestroyPipeline(vulkanApplication->vulkanDevice.logicalDevice, environment.graphicsPipeline, nullptr);
	//vkDestroyPipeline(vulkanDevice.logicalDevice, particleFire.graphicsPipeline, nullptr);
}

void FVulkanModelRenderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(vulkanApplication->vulkanDevice.logicalDevice, vulkanApplication->commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void FVulkanModelRenderer::CreateBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	environment.CreateBuffers(scene, vulkanDevice, commandPool, graphicsQueue);
	//particleFire.CreateBuffers(vulkanDevice);
	terrain.CreateBuffers(vulkanDevice, commandPool, graphicsQueue);
}

void  FVulkanModelRenderer::CreateDescriptorPool(FVulkanDevice vulkanDevice)
{
	std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {};
	descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSizes[0].descriptorCount = 3;
	descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSizes[1].descriptorCount = 6;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = FVulkanInitializers::DescriptorPoolCreateInfo();
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolInfo.pPoolSizes = descriptorPoolSizes.data();
	descriptorPoolInfo.maxSets = 3;

	if (vkCreateDescriptorPool(vulkanDevice.logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void  FVulkanModelRenderer::CreateDescriptorSets(FVulkanDevice vulkanDevice)
{
	environment.CreateDescriptorSets(vulkanDevice.logicalDevice, descriptorSetLayout, descriptorPool);
	//particleFire.CreateDescriptorSets(vulkanDevice.logicalDevice, descriptorSetLayout, descriptorPool);
	terrain.CreateDescriptorSets(vulkanDevice.logicalDevice, descriptorSetLayout, descriptorPool);
}

void  FVulkanModelRenderer::CreateCommandBuffers(FVulkanDevice vulkanDevice)
{
	commandBuffers.resize(vulkanApplication->swapChain.imageCount);

	VkCommandBufferAllocateInfo allocateInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	allocateInfo.commandPool = vulkanApplication->commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(vulkanDevice.logicalDevice, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void  FVulkanModelRenderer::BuildCommandBuffers(FVulkanDevice vulkanDevice)
{

	VkCommandBufferBeginInfo beginInfo = FVulkanInitializers::CommandBufferBeginInfo();
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = FVulkanInitializers::RenderPassBeginInfo();
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = vulkanApplication->swapChain.extent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		renderPassInfo.framebuffer = vulkanApplication->swapChain.frameBuffers[i];

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		environment.BuildCommandBuffers(commandBuffers[i], scene, vulkanApplication->pipelineLayout);
		//particleFire.BuildCommandBuffers(commandBuffers[i], scene, pipelineLayout);
		terrain.BuildCommandBuffers(commandBuffers[i], scene, vulkanApplication->pipelineLayout);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//void FVulkanModelRenderer::CreateSemaphores(FVulkanDevice vulkanDevice)
//{
//	VkSemaphoreCreateInfo semaphoreInfo = FVulkanInitializers::SemaphoreCreateInfo();
//
//	if (vkCreateSemaphore(vulkanDevice.logicalDevice, &semaphoreInfo, nullptr, &vulkanApplication->imageAvailableSemaphore) != VK_SUCCESS ||
//		vkCreateSemaphore(vulkanDevice.logicalDevice, &semaphoreInfo, nullptr, &vulkanApplication->renderFinishedSemaphore) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create semaphores!");
//	}
//}

void FVulkanModelRenderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uniformBufferObjectLayoutBinding = {};
	uniformBufferObjectLayoutBinding.binding = 0;
	uniformBufferObjectLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferObjectLayoutBinding.descriptorCount = 1;
	uniformBufferObjectLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uniformBufferObjectLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding sampler2LayoutBinding = {};
	sampler2LayoutBinding.binding = 2;
	sampler2LayoutBinding.descriptorCount = 1;
	sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler2LayoutBinding.pImmutableSamplers = nullptr;
	sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uniformBufferObjectLayoutBinding, samplerLayoutBinding, sampler2LayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = FVulkanInitializers::DescriptorSetLayoutCreateInfo();
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(vulkanApplication->vulkanDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void FVulkanModelRenderer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = vulkanApplication->swapChain.colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FVulkanCalculator::FindDepthFormat(vulkanApplication->vulkanDevice.physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;
	subPass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = FVulkanInitializers::RenderPassCreateInfo();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(vulkanApplication->vulkanDevice.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}