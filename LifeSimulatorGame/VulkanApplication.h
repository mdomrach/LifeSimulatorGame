#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Vertex.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "Cursor3D.h"
#include "Environment.h"
#include "ParticleFire.h"
#include "VulkanTerrain.h"

class FScene;
class FInputManager;
class FTimeManager;
class FGameManager;
class FVulkanScreenGrab;

class FVulkanApplication {
public:
	void Initialize(FGameManager* gameManager);

	void InitializeVulkan();

	void UpdateUniformBuffer();
	void DrawFrame();

	void Cleanup();

	struct GLFWwindow* window;
	
private:
	FScene* scene;
	
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	const std::vector<const char*> validationLayers = 
	{
		"VK_LAYER_LUNARG_standard_validation"
	};
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VkInstance instance;
	VkSurfaceKHR surface;

	void BuildCommandBuffers();
public:
	class FTextOverlay *textOverlay = nullptr;

	FVulkanDevice vulkanDevice;
//private:
	VkQueue graphicsQueue; 
	VkQueue presentQueue;

	FVulkanSwapChain swapChain;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkDescriptorPool descriptorPool;

	FVulkanCursor3D cursor3D;
	FEnvironment environment;
	//FParticleFire particleFire;
	FVulkanTerrain terrain;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	FInputManager* inputManager;
	FTimeManager* timeManager;
	FVulkanScreenGrab* screenGrab;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void InitWindow();
	VkResult CreateVulkanInstance();
	void CreateWindowSurface();
	void SetupDevice();

	void InitializeSwapChain();

	void PrepareToDisplayScene();
	void LoadScene();
	
	void GetDeviceQueues();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> avaliablePresentModes);
	VkExtent2D ChoseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();

	void CreateSwapChain();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateCommandPool();
	void CreateCommandBuffers();

	void CreateFrameBuffers();
	void CreateSemaphores();

	void CleanupSwapChain();
	void RecreateSwapChain();
	static void OnWindowResized(GLFWwindow* window, int width, int height);

	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDepthResources();


	FGameManager* gameManager;
};