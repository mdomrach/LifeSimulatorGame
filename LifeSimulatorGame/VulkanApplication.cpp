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
#include "VulkanTextOverlay.h"
#include <iosfwd>
#include "VulkanInitializers.h"
#include "VulkanFactory.h"
#include "VulkanCalculator.h"
#include "VulkanCommandBufferCalculator.h"
#include "VulkanImageCalculator.h"
#include "VulkanPipelineCalculator.h"
#include "FileCalculator.h"
#include "VulkanScreenGrab.h"
#include "VulkanModelRenderer.h"
#include "VulkanApplicationData.h"

void FVulkanApplication::Initialize(FGameManager* gameManager)
{
	this->gameManager = gameManager;
	inputManager = gameManager->inputManager;
	timeManager = gameManager->timeManager;
	scene = gameManager->scene;
	screenGrab = gameManager->screenGrab;
	applicationData = gameManager->applicationData;

	modelRenderer = new FVulkanModelRenderer();
	modelRenderer->Initialize(gameManager);
	textOverlay = new FVulkanTextOverlay();
	textOverlay->Initialize(gameManager);
	cursor3D.Initialize(gameManager);
}

void FVulkanApplication::InitializeVulkan()
{

	InitWindow();
	if (CreateVulkanInstance() != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
	CreateWindowSurface();

	SetupDevice();
	CreateSemaphores(applicationData->vulkanDevice);
	CreateCommandPool();

	modelRenderer->InitializeVulkan(applicationData->vulkanDevice);

	UpdateSwapChain();

	scene->camera = new FCamera(applicationData->swapChain.extent.width, applicationData->swapChain.extent.height);

}

void FVulkanApplication::UpdateSwapChain()
{
	CreateSwapChain();
	CreateRenderPass();
	CreateDepthResources();
	CreateFrameBuffers();

	modelRenderer->UpdateSwapChain(applicationData->vulkanDevice);
	textOverlay->UpdateSwapChain();
	cursor3D.UpdateSwapChain(gameManager);
	screenGrab->UpdateSwapChain(applicationData->vulkanDevice, this);
}

void FVulkanApplication::RecreateSwapChain()
{
	vkDeviceWaitIdle(applicationData->vulkanDevice.logicalDevice);

	CleanupSwapChain();

	UpdateSwapChain();
}

void FVulkanApplication::CreateSemaphores(FVulkanDevice vulkanDevice)
{
	VkSemaphoreCreateInfo semaphoreInfo = FVulkanInitializers::SemaphoreCreateInfo();

	if (vkCreateSemaphore(vulkanDevice.logicalDevice, &semaphoreInfo, nullptr, &applicationData->imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(vulkanDevice.logicalDevice, &semaphoreInfo, nullptr, &applicationData->renderFinishedSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create semaphores!");
	}
}

void FVulkanApplication::CreateCommandPool()
{
	FQueueFamilyIndices queueFamilyIndices = applicationData->vulkanDevice.queueFamilyIndices;

	VkCommandPoolCreateInfo poolInfo = FVulkanInitializers::CommandPoolCreateInfo();
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0;

	if (vkCreateCommandPool(applicationData->vulkanDevice.logicalDevice, &poolInfo, nullptr, &applicationData->commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}


void FVulkanApplication::InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, FVulkanApplication::OnWindowResized);

}

void FVulkanApplication::SetupDevice()
{
	FVulkanDeviceCreateInfo createInfo;
	createInfo.surface = surface;

	if (createInfo.Create(&instance, &applicationData->vulkanDevice) != VK_SUCCESS)
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
	auto& indicies = applicationData->vulkanDevice.queueFamilyIndices;
	vkGetDeviceQueue(applicationData->vulkanDevice.logicalDevice, indicies.graphicsFamily, 0, &applicationData->graphicsQueue);
	vkGetDeviceQueue(applicationData->vulkanDevice.logicalDevice, indicies.presentFamily, 0, &applicationData->presentQueue);
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
	screenGrab->Destroy(applicationData->vulkanDevice);
	cursor3D.Destroy(applicationData->vulkanDevice);
	textOverlay->Destroy();


	vkDestroyImageView(applicationData->vulkanDevice.logicalDevice, applicationData->depthImageView, nullptr);
	vkDestroyImage(applicationData->vulkanDevice.logicalDevice, applicationData->depthImage, nullptr);
	vkFreeMemory(applicationData->vulkanDevice.logicalDevice, applicationData->depthImageMemory, nullptr);

	for (size_t i = 0; i < applicationData->swapChain.imageCount; i++)
	{
		vkDestroyFramebuffer(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.frameBuffers[i], nullptr);
	}

	modelRenderer->FreeCommandBuffers();

	modelRenderer->DestroyPipelines();

	vkDestroyRenderPass(applicationData->vulkanDevice.logicalDevice, applicationData->renderPass, nullptr);
	for (size_t i = 0; i < applicationData->swapChain.imageCount; i++)
	{
		vkDestroyImageView(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.swapChain, nullptr);
}

void FVulkanApplication::Cleanup()
{
	CleanupSwapChain();

	modelRenderer->Destroy(applicationData->vulkanDevice);


	//modelRenderer->DestroyBuffers(vulkanDevice);

	vkDestroySemaphore(applicationData->vulkanDevice.logicalDevice, applicationData->renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(applicationData->vulkanDevice.logicalDevice, applicationData->imageAvailableSemaphore, nullptr);

	vkDestroyCommandPool(applicationData->vulkanDevice.logicalDevice, applicationData->commandPool, nullptr);

	applicationData->vulkanDevice.Destroy(instance);
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
	FSwapChainSupportDetails swapChainSupport = applicationData->vulkanDevice.swapChainSupportDetails;

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

	FQueueFamilyIndices indices = applicationData->vulkanDevice.queueFamilyIndices;
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

	if (vkCreateSwapchainKHR(applicationData->vulkanDevice.logicalDevice, &createInfo, nullptr, &applicationData->swapChain.swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	applicationData->swapChain.imageCount = imageCount;
	vkGetSwapchainImagesKHR(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.swapChain, &imageCount, nullptr);
	applicationData->swapChain.SetImageCount(imageCount);
	vkGetSwapchainImagesKHR(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.swapChain, &imageCount, applicationData->swapChain.images.data());

	applicationData->swapChain.colorFormat = surfaceFormat.format;
	applicationData->swapChain.extent = extent;

	for (size_t i = 0; i < applicationData->swapChain.imageCount; i++)
	{
		applicationData->swapChain.imageViews[i] = FVulkanFactory::ImageView(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.images[i], applicationData->swapChain.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void FVulkanApplication::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = applicationData->swapChain.colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FVulkanCalculator::FindDepthFormat(applicationData->vulkanDevice.physicalDevice);
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

	if (vkCreateRenderPass(applicationData->vulkanDevice.logicalDevice, &renderPassInfo, nullptr, &applicationData->renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void FVulkanApplication::CreateFrameBuffers()
{
	for (size_t i = 0; i < applicationData->swapChain.imageCount; i++)
	{
		std::array<VkImageView, 2> attachments = {
			applicationData->swapChain.imageViews[i],
			applicationData->depthImageView
		};

		VkFramebufferCreateInfo frameBufferInfo = FVulkanInitializers::FramebufferCreateInfo();
		frameBufferInfo.renderPass = applicationData->renderPass;
		frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferInfo.pAttachments = attachments.data();
		frameBufferInfo.width = applicationData->swapChain.extent.width;
		frameBufferInfo.height = applicationData->swapChain.extent.height;
		frameBufferInfo.layers = 1;

		if (vkCreateFramebuffer(applicationData->vulkanDevice.logicalDevice, &frameBufferInfo, nullptr, &applicationData->swapChain.frameBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void FVulkanApplication::DrawFrame()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(applicationData->vulkanDevice.logicalDevice, applicationData->swapChain.swapChain, std::numeric_limits<uint64_t>::max(), applicationData->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	modelRenderer->Submit(applicationData->graphicsQueue, imageIndex);
	screenGrab->Submit(applicationData->graphicsQueue, imageIndex);
	cursor3D.Submit(applicationData->graphicsQueue, imageIndex);
	textOverlay->Submit(applicationData->graphicsQueue, imageIndex);
	

	VkSemaphore signalSemaphores[] = { applicationData->renderFinishedSemaphore };
	VkPresentInfoKHR presentInfo = FVulkanInitializers::PresentInfoKHR();
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { applicationData->swapChain.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(applicationData->presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkDeviceWaitIdle(applicationData->vulkanDevice.logicalDevice);
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

void FVulkanApplication::UpdateFrame()
{
	modelRenderer->UpdateFrame(applicationData->vulkanDevice.logicalDevice);
}

void FVulkanApplication::CreateDepthResources()
{
	VkFormat depthFormat = FVulkanCalculator::FindDepthFormat(applicationData->vulkanDevice.physicalDevice);
	FVulkanImageCalculator::CreateImage(applicationData->vulkanDevice, applicationData->swapChain.extent.width, applicationData->swapChain.extent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		applicationData->depthImage, applicationData->depthImageMemory);
	applicationData->depthImageView = FVulkanFactory::ImageView(applicationData->vulkanDevice.logicalDevice, applicationData->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	FVulkanImageCalculator::TransitionImageLayout(
		applicationData->vulkanDevice.logicalDevice, applicationData->commandPool, applicationData->graphicsQueue,
		applicationData->depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}