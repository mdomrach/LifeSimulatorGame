#include "VulkanDevice.h"
#include <iostream>
#include <set>
#include "SwapChainSupportDetails.h"
#include "VulkanInitializers.h"


VkResult FVulkanDeviceCreateInfo::Create(VkInstance* instance, FVulkanDevice* vulkanDevice)
{
	if (enableValidationLayers)
	{
		VkResult result = SetupDebugCallback(instance, vulkanDevice);
		//std::cout << result;
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug callback!");
		}
	}

	std::vector<VkPhysicalDevice> Devices;
	if (TryFindVulkanGPUs(instance, Devices) != true)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	if (PickPhysicalDevice(instance, Devices, vulkanDevice) != true)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vulkanDevice->queueFamilyIndices = FindQueueFamilies(vulkanDevice->physicalDevice);
	vulkanDevice->swapChainSupportDetails = QuerySwapChainSupport(vulkanDevice->physicalDevice);
	if (CreateLogicalDevice(instance, vulkanDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	return VK_SUCCESS;
}


VkResult FVulkanDeviceCreateInfo::SetupDebugCallback(VkInstance* instance, FVulkanDevice* vulkanDevice)// VkDebugReportCallbackEXT& callback)
{
	VkDebugReportCallbackCreateInfoEXT createInfo = FVulkanInitializers::DebugReportCallbackCreateInfoEXT();
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback;

	return CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &vulkanDevice->callback);
}

VKAPI_ATTR VkBool32 VKAPI_CALL FVulkanDeviceCreateInfo::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	std::cerr << "validation layer: " << msg << std::endl;
	return VK_FALSE;
}

bool FVulkanDeviceCreateInfo::TryFindVulkanGPUs(VkInstance* instance, std::vector<VkPhysicalDevice>& devices)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		return false;
	}

	devices = std::vector<VkPhysicalDevice>(deviceCount);
	vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());
	return true;
}

bool FVulkanDeviceCreateInfo::PickPhysicalDevice(VkInstance* instance, const std::vector<VkPhysicalDevice>& devices, FVulkanDevice* vulkanDevice)
{
	int bestDeviceSuitability = 0;
	for (const auto& Device : devices)
	{
		int deviceSuitability = RateDeviceSuitability(Device);
		if (vulkanDevice->physicalDevice == VK_NULL_HANDLE || deviceSuitability > bestDeviceSuitability)
		{
			vulkanDevice->physicalDevice = Device;
			bestDeviceSuitability = deviceSuitability;
		}
	}

	return vulkanDevice->physicalDevice != VK_NULL_HANDLE && bestDeviceSuitability > 0;
}

VkResult FVulkanDeviceCreateInfo::CreateLogicalDevice(VkInstance* instance, FVulkanDevice* vulkanDevice)
{
	auto indicies = vulkanDevice->queueFamilyIndices;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indicies.graphicsFamily, indicies.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = FVulkanInitializers::DeviceQueueCreateInfo();
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = FVulkanInitializers::DeviceCreateInfo();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateDevice(vulkanDevice->physicalDevice, &createInfo, nullptr, &vulkanDevice->logicalDevice);
	//TODO: Get Device Queue Later
	if (result != VK_SUCCESS)
	{
		return result;
	}
	//vkGetDeviceQueue(logicalDevice, indicies.graphicsFamily, 0, &graphicsQueue);
	//vkGetDeviceQueue(logicalDevice, indicies.presentFamily, 0, &presentQueue);
	return result;
}


VkResult FVulkanDeviceCreateInfo::CreateDebugReportCallbackEXT(VkInstance* instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(*instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

int FVulkanDeviceCreateInfo::RateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	int Score = 0;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		Score += 1000;
	}

	Score += deviceProperties.limits.maxImageDimension2D;
	if (!supportedFeatures.geometryShader && supportedFeatures.samplerAnisotropy)
	{
		return 0;
	}

	auto queueFamilyIndices = FindQueueFamilies(device);
	if (!queueFamilyIndices.IsComplete())
	{
		return 0;
	}

	if (!CheckDeviceExtensionSupport(device))
	{
		return 0;
	}

	auto swapChainSupport = QuerySwapChainSupport(device);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
	{
		return 0;
	}

	return Score;
}

FQueueFamilyIndices FVulkanDeviceCreateInfo::FindQueueFamilies(VkPhysicalDevice device)
{
	FQueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount && !indices.IsComplete(); i++)
	{
		auto queueFamily = queueFamilies[i];
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 PresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &PresentSupport);
		if (queueFamily.queueCount > 0 && PresentSupport)
		{
			indices.presentFamily = i;
		}
	}

	return indices;
}

bool FVulkanDeviceCreateInfo::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

FSwapChainSupportDetails FVulkanDeviceCreateInfo::QuerySwapChainSupport(VkPhysicalDevice device)
{
	FSwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void FVulkanDevice::Destroy(VkInstance instance)
{
	vkDestroyDevice(logicalDevice, nullptr);
	DestroyDebugReportCallbackEXT(instance, callback, nullptr);
}

void FVulkanDevice::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}