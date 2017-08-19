#include "VulkanApplication.h"

#include <stdexcept>
#include <functional>
#include <iostream>
#include <cstring>
#include "QueueFamilyIndices.h"
#include <set>
#include "SwapChainSupportDetails.h"
#include <algorithm>
#include "UniformBufferObject.h"
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <chrono>

#include "SceneCalculator.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"
#include "SleepCalculator.h"
#include "InputManager.h"
#include "GameManager.h"
#include "TimeManager.h"
#include "Terrain.h"
#include "VulkanDevice.h"
#include "TextOverlay.h"
#include <iosfwd>
#include "VulkanInitializers.h"
#include "VulkanFactory.h"
#include "VulkanCalculator.h"
#include "VulkanCommandBufferCalculator.h"
#include "VulkanImageCalculator.h"
#include "Environment.h"
#include "VulkanPipelineCalculator.h"
#include "FileCalculator.h"
#include "ShaderCalculator.h"
#include "VulkanBufferCalculator.h"
#include "VulkanScreenGrab.h"

void FVulkanApplication::InitializeVulkan()
{
	frameCount = 0;

	InitWindow();
	if (CreateVulkanInstance() != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
	CreateWindowSurface();

	SetupDevice();
	CreateDescriptorSetLayout();
	CreateCommandPool();

	InitializeSwapChain();

	LoadScene();
	PrepareToDisplayScene();
}

void FVulkanApplication::Initialize(FGameManager* gameManager)
{
	inputManager = gameManager->inputManager;
	timeManager = gameManager->timeManager;
	scene = gameManager->scene;
	screenGrab = gameManager->screenGrab;

	//particleFire.Initialize(gameManager);
	terrain.Initialize(gameManager);
}

void FVulkanApplication::InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, FVulkanApplication::OnWindowResized);

}

void FVulkanApplication::InitializeSwapChain()
{
	CreateSwapChain();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateDepthResources();
	CreateFrameBuffers();
}

void FVulkanApplication::LoadScene()
{
	//environment.LoadAssets(vulkanDevice, commandPool, graphicsQueue);
	//particleFire.LoadAssets(vulkanDevice, commandPool, graphicsQueue);
	terrain.LoadAssets();
	FSceneCalculator::LoadScene(scene, swapChain.extent.width, swapChain.extent.height);
}

void FVulkanApplication::PrepareToDisplayScene()
{
	environment.CreateBuffers(scene, vulkanDevice, commandPool, graphicsQueue);
	//particleFire.CreateBuffers(vulkanDevice);
	terrain.CreateBuffers(vulkanDevice, commandPool, graphicsQueue);
	CreateDescriptorPool();

	environment.CreateDescriptorSets(vulkanDevice.logicalDevice, descriptorSetLayout, descriptorPool);
	//particleFire.CreateDescriptorSets(vulkanDevice.logicalDevice, descriptorSetLayout, descriptorPool);
	terrain.CreateDescriptorSets(vulkanDevice.logicalDevice, descriptorSetLayout, descriptorPool);
	CreateCommandBuffers();
	BuildCommandBuffers();
	PrepareTextOverlay();
	screenGrab->CreateCommandBuffers(vulkanDevice, commandPool, swapChain);
	screenGrab->BuildCommandBuffers(vulkanDevice, commandPool, swapChain, depthImage, presentQueue);
	CreateSemaphores();
}

void FVulkanApplication::SetupDevice()
{
	FVulkanDeviceCreateInfo createInfo;
	createInfo.surface = surface;

	if (createInfo.Create(&instance, &vulkanDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create device!");
	}

	GetDeviceQueues();
}

VkResult FVulkanApplication::CreateVulkanInstance()
{
	VkApplicationInfo applicationInfo = FVulkanInitializers::ApplicationInfo();
	applicationInfo.pApplicationName = "Life Simulation Game";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo createInfo = FVulkanInitializers::InstanceCreateInfo();
	createInfo.pApplicationInfo = &applicationInfo;

	auto Extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
	createInfo.ppEnabledExtensionNames = Extensions.data();

	if (enableValidationLayers)
	{
		if (!CheckValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	return vkCreateInstance(&createInfo, nullptr, &instance);
}

void FVulkanApplication::CreateWindowSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void FVulkanApplication::GetDeviceQueues()
{
	auto& indicies = vulkanDevice.queueFamilyIndices;
	vkGetDeviceQueue(vulkanDevice.logicalDevice, indicies.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(vulkanDevice.logicalDevice, indicies.presentFamily, 0, &presentQueue);
}

VkSurfaceFormatKHR FVulkanApplication::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}



VkPresentModeKHR FVulkanApplication::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> avaliablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : avaliablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D FVulkanApplication::ChoseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void FVulkanApplication::CleanupSwapChain()
{
	screenGrab->Destroy(vulkanDevice);

	textOverlay->Destroy(&vulkanDevice);

	vkDestroyImageView(vulkanDevice.logicalDevice, depthImageView, nullptr);
	vkDestroyImage(vulkanDevice.logicalDevice, depthImage, nullptr);
	vkFreeMemory(vulkanDevice.logicalDevice, depthImageMemory, nullptr);

	for (size_t i = 0; i < swapChain.imageCount; i++)
	{
		vkDestroyFramebuffer(vulkanDevice.logicalDevice, swapChain.frameBuffers[i], nullptr);
	}

	vkFreeCommandBuffers(vulkanDevice.logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkDestroyPipeline(vulkanDevice.logicalDevice, environment.graphicsPipeline, nullptr);
	//vkDestroyPipeline(vulkanDevice.logicalDevice, particleFire.graphicsPipeline, nullptr);
	vkDestroyPipeline(vulkanDevice.logicalDevice, terrain.graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(vulkanDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(vulkanDevice.logicalDevice, renderPass, nullptr);
	for (size_t i = 0; i < swapChain.imageCount; i++)
	{
		vkDestroyImageView(vulkanDevice.logicalDevice, swapChain.imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(vulkanDevice.logicalDevice, swapChain.swapChain, nullptr);
}

void FVulkanApplication::Cleanup()
{
	CleanupSwapChain();

	environment.Destroy(vulkanDevice);
	//particleFire.Destroy(vulkanDevice);

	vkDestroyDescriptorPool(vulkanDevice.logicalDevice, descriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(vulkanDevice.logicalDevice, descriptorSetLayout, nullptr);

	environment.DestroyBuffers(vulkanDevice);
	//particleFire.DestroyBuffers(vulkanDevice);
	terrain.DestroyBuffers(vulkanDevice);

	vkDestroySemaphore(vulkanDevice.logicalDevice, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(vulkanDevice.logicalDevice, imageAvailableSemaphore, nullptr);

	vkDestroyCommandPool(vulkanDevice.logicalDevice, commandPool, nullptr);

	vulkanDevice.Destroy(instance);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

bool FVulkanApplication::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> FVulkanApplication::GetRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwWExtensions;
	glfwWExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwWExtensions[i]);
	}

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

void FVulkanApplication::CreateSwapChain()
{
	FSwapChainSupportDetails swapChainSupport = vulkanDevice.swapChainSupportDetails;

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChoseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = FVulkanInitializers::SwapchainCreateInfoKHR();
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	FQueueFamilyIndices indices = vulkanDevice.queueFamilyIndices;
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(vulkanDevice.logicalDevice, &createInfo, nullptr, &swapChain.swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	swapChain.imageCount = imageCount;
	vkGetSwapchainImagesKHR(vulkanDevice.logicalDevice, swapChain.swapChain, &imageCount, nullptr);
	swapChain.SetImageCount(imageCount);
	vkGetSwapchainImagesKHR(vulkanDevice.logicalDevice, swapChain.swapChain, &imageCount, swapChain.images.data());

	swapChain.colorFormat = surfaceFormat.format;
	swapChain.extent = extent;

	for (size_t i = 0; i < swapChain.imageCount; i++)
	{
		swapChain.imageViews[i] = FVulkanFactory::ImageView(vulkanDevice.logicalDevice, swapChain.images[i], swapChain.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void FVulkanApplication::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChain.colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FVulkanCalculator::FindDepthFormat(vulkanDevice.physicalDevice);
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

	if (vkCreateRenderPass(vulkanDevice.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void FVulkanApplication::CreateDescriptorSetLayout()
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

	if (vkCreateDescriptorSetLayout(vulkanDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void FVulkanApplication::CreateGraphicsPipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = FVulkanInitializers::PipelineLayoutCreateInfo();
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(vulkanDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo* pipelineInfo = FVulkanPipelineCalculator::CreateGraphicsPipelineInfo(swapChain, descriptorSetLayout, vulkanDevice.logicalDevice, renderPass, pipelineLayout);

	environment.PreparePipeline(vulkanDevice.logicalDevice, pipelineInfo);
	//particleFire.PreparePipeline(vulkanDevice.logicalDevice, pipelineInfo);
	terrain.PreparePipeline(vulkanDevice.logicalDevice, pipelineInfo);

	FVulkanPipelineCalculator::DeleteGraphicsPipelineInfo(pipelineInfo);
}

void FVulkanApplication::CreateCommandPool()
{
	FQueueFamilyIndices queueFamilyIndices = vulkanDevice.queueFamilyIndices;

	VkCommandPoolCreateInfo poolInfo = FVulkanInitializers::CommandPoolCreateInfo();
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0;

	if (vkCreateCommandPool(vulkanDevice.logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void FVulkanApplication::CreateCommandBuffers()
{
	commandBuffers.resize(swapChain.imageCount);

	VkCommandBufferAllocateInfo allocateInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(vulkanDevice.logicalDevice, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void FVulkanApplication::BuildCommandBuffers()
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
	renderPassInfo.renderArea.extent = swapChain.extent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		renderPassInfo.framebuffer = swapChain.frameBuffers[i];

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		environment.BuildCommandBuffers(commandBuffers[i], scene, pipelineLayout);
		//particleFire.BuildCommandBuffers(commandBuffers[i], scene, pipelineLayout);
		terrain.BuildCommandBuffers(commandBuffers[i], scene, pipelineLayout);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void FVulkanApplication::CreateFrameBuffers()
{
	for (size_t i = 0; i < swapChain.imageCount; i++)
	{
		std::array<VkImageView, 2> attachments = {
			swapChain.imageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo frameBufferInfo = FVulkanInitializers::FramebufferCreateInfo();
		frameBufferInfo.renderPass = renderPass;
		frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferInfo.pAttachments = attachments.data();
		frameBufferInfo.width = swapChain.extent.width;
		frameBufferInfo.height = swapChain.extent.height;
		frameBufferInfo.layers = 1;

		if (vkCreateFramebuffer(vulkanDevice.logicalDevice, &frameBufferInfo, nullptr, &swapChain.frameBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void FVulkanApplication::DrawFrame()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(vulkanDevice.logicalDevice, swapChain.swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	screenGrab->Submit(graphicsQueue, imageIndex);
	textOverlay->Submit(graphicsQueue, imageIndex);

	VkPresentInfoKHR presentInfo = FVulkanInitializers::PresentInfoKHR();
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkDeviceWaitIdle(vulkanDevice.logicalDevice);
}

void FVulkanApplication::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = FVulkanInitializers::SemaphoreCreateInfo();

	if (vkCreateSemaphore(vulkanDevice.logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(vulkanDevice.logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create semaphores!");
	}
}

void FVulkanApplication::RecreateSwapChain()
{
	vkDeviceWaitIdle(vulkanDevice.logicalDevice);

	CleanupSwapChain();

	CreateSwapChain();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateDepthResources();
	CreateFrameBuffers();
	CreateCommandBuffers();
	BuildCommandBuffers();
	PrepareTextOverlay();
	screenGrab->CreateCommandBuffers(vulkanDevice, commandPool, swapChain);
	screenGrab->BuildCommandBuffers(vulkanDevice, commandPool, swapChain, depthImage, presentQueue);
}

void FVulkanApplication::OnWindowResized(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0)
	{
		return;
	}

	FVulkanApplication* app = reinterpret_cast<FVulkanApplication*>(glfwGetWindowUserPointer(window));
	app->RecreateSwapChain();
}

void FVulkanApplication::UpdateUniformBuffer()
{
	environment.UpdateUniformBuffer(vulkanDevice.logicalDevice, scene);
	//particleFire.UpdateUniformBuffer(vulkanDevice.logicalDevice, scene);
	//particleFire.UpdateParticles();
	terrain.UpdateFrame(vulkanDevice.logicalDevice, scene);

	frameCount++;
	if (timeManager->startFrameTime > nextFPSUpdateTime)
	{
		UpdateTextOverlay();
		frameCount = 0;
		nextFPSUpdateTime++;
	}
}

void FVulkanApplication::CreateDescriptorPool()
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

void FVulkanApplication::CreateDepthResources()
{
	VkFormat depthFormat = FVulkanCalculator::FindDepthFormat(vulkanDevice.physicalDevice);
	FVulkanImageCalculator::CreateImage(vulkanDevice, swapChain.extent.width, swapChain.extent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage, depthImageMemory);
	depthImageView = FVulkanFactory::ImageView(vulkanDevice.logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	FVulkanImageCalculator::TransitionImageLayout(
		vulkanDevice.logicalDevice, commandPool, graphicsQueue,
		depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

// Update the text buffer displayed by the text overlay
void FVulkanApplication::UpdateTextOverlay()
{
	textOverlay->BeginTextUpdate(&vulkanDevice);

	//textOverlay->AddText("Life Simulator Game", 5.0f, 5.0f, FTextOverlay::alignLeft);

	//int fps = frameCount;
	//float timeFor60Frames = 60.0f / fps;
	//std::string timeText = std::to_string(timeFor60Frames);

	int fps = frameCount;
	std::string timeText = std::to_string(fps);
	textOverlay->AddText(timeText, 5.0f, 5.0f, FTextOverlay::alignLeft);
	//textOverlay->AddText(title, 5.0f, 5.0f, FTextOverlay::alignLeft);

	//std::stringstream ss;
	//ss << std::fixed << std::setprecision(2) << (frameTimer * 1000.0f) << "ms (" << lastFPS << " fps)";
	//textOverlay->AddText(ss.str(), 5.0f, 25.0f, ETextOverlay::alignLeft);

	//textOverlay->AddText(deviceProperties.deviceName, 5.0f, 45.0f, FTextOverlay::alignLeft);

	//textOverlay->AddText("Press \"space\" to toggle text overlay", 5.0f, 65.0f, FTextOverlay::alignLeft);
	//textOverlay->AddText("Hold middle mouse button and drag to move", 5.0f, 85.0f, FTextOverlay::alignLeft);

	textOverlay->EndTextUpdate(&vulkanDevice);
}

void FVulkanApplication::PrepareTextOverlay()
{
	// Load the text rendering shaders

	auto vertShaderCode = FFileCalculator::ReadFile("shaders/text.vert.spv");
	auto fragShaderCode = FFileCalculator::ReadFile("shaders/text.frag.spv");

	VkShaderModule vertShaderModule = FShaderCalculator::CreateShaderModule(vulkanDevice.logicalDevice, vertShaderCode);
	VkShaderModule fragShaderModule = FShaderCalculator::CreateShaderModule(vulkanDevice.logicalDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = FVulkanInitializers::PipelineShaderStageCreateInfo();
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> textshaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	textOverlay = new FTextOverlay();
	textOverlay->Initialize(this, textshaderStages);

	UpdateTextOverlay();

	vkDestroyShaderModule(vulkanDevice.logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(vulkanDevice.logicalDevice, fragShaderModule, nullptr);
}