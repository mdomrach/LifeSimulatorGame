#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Vertex.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "Cursor3D.h"
//#include "Environment.h"
//#include "ParticleFire.h"
//#include "VulkanTerrain.h"

class FScene;
class FInputManager;
class FTimeManager;
class FGameManager;
class FVulkanScreenGrab;
class FVulkanModelRenderer;

class FVulkanApplication {
public:
	void Initialize(FGameManager* gameManager);

	void InitializeVulkan();

	void UpdateFrame();
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

public:
	class FVulkanApplicationData* applicationData;
	class FVulkanTextOverlay *textOverlay = nullptr;

	FVulkanCursor3D cursor3D;

	FInputManager* inputManager;
	FTimeManager* timeManager;
	FVulkanScreenGrab* screenGrab;
	FVulkanModelRenderer* modelRenderer;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void InitWindow();
	VkResult CreateVulkanInstance();
	void CreateWindowSurface();
	void SetupDevice();

	void UpdateSwapChain();
	void CreateCommandPool();
	void CreateSemaphores(FVulkanDevice vulkanDevice);
	void CreateRenderPass();

	void GetDeviceQueues();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> avaliablePresentModes);
	VkExtent2D ChoseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();

	void CreateSwapChain();

	void CreateFrameBuffers();

	void CleanupSwapChain();
	void RecreateSwapChain();
	static void OnWindowResized(GLFWwindow* window, int width, int height);

	void CreateDepthResources();

	FGameManager* gameManager;
};