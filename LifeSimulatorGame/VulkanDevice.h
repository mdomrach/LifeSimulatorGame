#pragma once

#include <vulkan/vulkan.h>
#include "QueueFamilyIndices.h"
#include <vector>
#include "SwapChainSupportDetails.h"

class FVulkanDevice
{
public:
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice = VK_NULL_HANDLE;
	//VkPhysicalDeviceProperties properties;
	//VkPhysicalDeviceFeatures features;
	//VkPhysicalDeviceFeatures enabledFeatures;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);
	//std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	//std::vector<std::string> supportedExtensions;
	FSwapChainSupportDetails swapChainSupportDetails;
	FQueueFamilyIndices queueFamilyIndices;
	VkDebugReportCallbackEXT callback;

	void Destroy(VkInstance instance);
private:
	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class FVulkanDeviceCreateInfo
{
public:
	VkSurfaceKHR surface;

	VkResult Create(VkInstance* instance, FVulkanDevice* vulkanDevice);
private:
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	VkResult SetupDebugCallback(VkInstance* instance, FVulkanDevice* vulkanDevice);
	
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData);
	bool TryFindVulkanGPUs(VkInstance* instance, std::vector<VkPhysicalDevice>& devices);
	bool PickPhysicalDevice(VkInstance* instance, const std::vector<VkPhysicalDevice>& devices, FVulkanDevice* vulkanDevice);
	VkResult CreateLogicalDevice(VkInstance* instance, FVulkanDevice* vulkanDevice);
	VkResult CreateDebugReportCallbackEXT(VkInstance* instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	int RateDeviceSuitability(VkPhysicalDevice device);
	FQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	struct FSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
};
