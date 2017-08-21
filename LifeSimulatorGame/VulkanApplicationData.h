#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
//#include "Vertex.h"
#include "VulkanSwapChain.h"
//#include "VulkanBuffer.h"
#include "VulkanDevice.h"
//#include "Cursor3D.h"

//class FScene;
//class FInputManager;
//class FTimeManager;
//class FGameManager;
//class FVulkanScreenGrab;
//class FVulkanModelRenderer;

class FVulkanApplicationData {
public:
	FVulkanDevice vulkanDevice;
	//private:
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	FVulkanSwapChain swapChain;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;

	VkCommandPool commandPool;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	//FVulkanCursor3D cursor3D;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
};