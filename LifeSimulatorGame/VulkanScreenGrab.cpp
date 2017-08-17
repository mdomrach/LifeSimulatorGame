#include "VulkanScreenGrab.h"

#include "VulkanCommandBufferCalculator.h"
#include "VulkanBufferCalculator.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanApplication.h"

#include "VulkanCalculator.h"
#include "VulkanImageCalculator.h"
#include "VulkanFactory.h"
#include "VulkanInitializers.h"

#include <fstream>
#include <iostream>

#include "GameManager.h"
#include "InputManager.h"

//#define GLM_FORCE_RADIANS
//#include <glm/glm.hpp>

void FVulkanScreenGrab::Initialize(FGameManager* gameManager)
{
	inputManager = gameManager->inputManager;
	vulkanApplication = gameManager->vulkanApplication;
	inputManager->MonitorKeyState(GLFW_KEY_P);
	isScreenShot = false;
}

void FVulkanScreenGrab::ProcessInput()
{
	auto keyState = inputManager->GetKeyState(GLFW_KEY_P);
	if (keyState && !isScreenShot)
	{
		//GrabDepthUsingBuffer(vulkanApplication->vulkanDevice, vulkanApplication->swapChain, vulkanApplication->commandPool, vulkanApplication->presentQueue, vulkanApplication->swapChain.images[0], "Depth.ppm");
		GrabDepth(vulkanApplication->vulkanDevice, vulkanApplication->swapChain, vulkanApplication->commandPool, vulkanApplication->presentQueue, vulkanApplication->depthImage, "Depth.ppm");
		//GrabScreen(vulkanApplication->vulkanDevice, vulkanApplication->swapChain, vulkanApplication->commandPool, vulkanApplication->presentQueue, vulkanApplication->swapChain.images[0], "SwapChain.ppm");

		isScreenShot = true;
	}
	else if (!keyState && isScreenShot)
	{
		isScreenShot = false;
	}
}

void FVulkanScreenGrab::GrabDepth(FVulkanDevice vulkanDevice, FVulkanSwapChain swapChain, VkCommandPool commandPool, VkQueue queue, VkImage srcImage, const char* filename)
{
	VkImage screenImage;
	VkDeviceMemory screenImageMemory;

	int width = swapChain.extent.width;
	int height = swapChain.extent.height;

	// Create the linear tiled destination image to copy to and to read the memory from
	VkImageCreateInfo imgCreateInfo = FVulkanInitializers::ImageCreateInfo();// (vks::initializers::imageCreateInfo());
	imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
	imgCreateInfo.format = FVulkanCalculator::FindDepthFormat(vulkanDevice.physicalDevice);
	imgCreateInfo.extent.width = width;
	imgCreateInfo.extent.height = height;
	imgCreateInfo.extent.depth = 1;
	imgCreateInfo.arrayLayers = 1;
	imgCreateInfo.mipLevels = 1;
	imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	// Create the image
	//VK_CHECK_RESULT(vkCreateImage(device, &imgCreateInfo, nullptr, &dstImage));
	vkCreateImage(vulkanDevice.logicalDevice, &imgCreateInfo, nullptr, &screenImage);
	// Create memory to back up the image
	VkMemoryRequirements memRequirements;
	VkMemoryAllocateInfo memAllocInfo = FVulkanInitializers::MemoryAllocateInfo();
	vkGetImageMemoryRequirements(vulkanDevice.logicalDevice, screenImage, &memRequirements);
	memAllocInfo.allocationSize = memRequirements.size;
	// Memory must be host visible to copy from
	memAllocInfo.memoryTypeIndex = vulkanDevice.GetMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(vulkanDevice.logicalDevice, &memAllocInfo, nullptr, &screenImageMemory);
	vkBindImageMemory(vulkanDevice.logicalDevice, screenImage, screenImageMemory, 0);

	// Do the actual blit from the swapchain image to our host visible destination image
	VkCommandBuffer copyCmd = FVulkanCommandBufferCalculator::BeginSingleTimeCommands(vulkanDevice.logicalDevice, commandPool);

	VkImageMemoryBarrier imageMemoryBarrier = FVulkanInitializers::ImageMemoryBarrier();

	// Transition destination image to transfer destination layout
	InsertImageMemoryBarrier(
		copyCmd,
		screenImage,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	// Transition swapchain image from present to transfer source layout
	InsertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	// Otherwise use image copy (requires us to manually flip components)
	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width = width;
	imageCopyRegion.extent.height = height;
	imageCopyRegion.extent.depth = 1;

	// Issue the copy command
	vkCmdCopyImage(
		copyCmd,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		screenImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);

	// Transition destination image to general layout, which is the required layout for mapping the image memory later on
	InsertImageMemoryBarrier(
		copyCmd,
		screenImage,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	// Transition back the swap chain image after the blit is done
	InsertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	FVulkanCommandBufferCalculator::EndSingleTimeCommands(copyCmd, vulkanDevice.logicalDevice, queue, commandPool);

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{};
	subResource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	VkSubresourceLayout subResourceLayout;

	vkGetImageSubresourceLayout(vulkanDevice.logicalDevice, screenImage, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	vkMapMemory(vulkanDevice.logicalDevice, screenImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	std::ofstream file(filename, std::ios::out | std::ios::binary);

	// ppm header
	file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

	// ppm binary pixel data
	for (uint32_t y = 0; y < height; y++)
	{
		unsigned int *row = (unsigned int*)data;
		for (uint32_t x = 0; x < width; x++)
		{
			file.write((char*)row, 3);
			row++;
		}
		data += subResourceLayout.rowPitch;
	}
	file.close();

	std::cout << "Depth Screenshot saved to disk" << std::endl;
}

void FVulkanScreenGrab::GrabScreen(FVulkanDevice vulkanDevice, FVulkanSwapChain swapChain, VkCommandPool commandPool, VkQueue queue, VkImage srcImage, const char* filename)
{
	VkImage screenImage;
	VkDeviceMemory screenImageMemory;

	int width = swapChain.extent.width;
	int height = swapChain.extent.height;
	
	// Create the linear tiled destination image to copy to and to read the memory from
	VkImageCreateInfo imgCreateInfo = FVulkanInitializers::ImageCreateInfo();// (vks::initializers::imageCreateInfo());
	imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
	imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imgCreateInfo.extent.width = width;
	imgCreateInfo.extent.height = height;
	imgCreateInfo.extent.depth = 1;
	imgCreateInfo.arrayLayers = 1;
	imgCreateInfo.mipLevels = 1;
	imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	// Create the image
	//VK_CHECK_RESULT(vkCreateImage(device, &imgCreateInfo, nullptr, &dstImage));
	vkCreateImage(vulkanDevice.logicalDevice, &imgCreateInfo, nullptr, &screenImage);
	// Create memory to back up the image
	VkMemoryRequirements memRequirements;
	VkMemoryAllocateInfo memAllocInfo = FVulkanInitializers::MemoryAllocateInfo();
	vkGetImageMemoryRequirements(vulkanDevice.logicalDevice, screenImage, &memRequirements);
	memAllocInfo.allocationSize = memRequirements.size;
	// Memory must be host visible to copy from
	memAllocInfo.memoryTypeIndex = vulkanDevice.GetMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(vulkanDevice.logicalDevice, &memAllocInfo, nullptr, &screenImageMemory);
	vkBindImageMemory(vulkanDevice.logicalDevice, screenImage, screenImageMemory, 0);

	// Do the actual blit from the swapchain image to our host visible destination image
	VkCommandBuffer copyCmd = FVulkanCommandBufferCalculator::BeginSingleTimeCommands(vulkanDevice.logicalDevice, commandPool);

	VkImageMemoryBarrier imageMemoryBarrier = FVulkanInitializers::ImageMemoryBarrier();

	// Transition destination image to transfer destination layout
	InsertImageMemoryBarrier(
		copyCmd,
		screenImage,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// Transition swapchain image from present to transfer source layout
	InsertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// Otherwise use image copy (requires us to manually flip components)
	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width = width;
	imageCopyRegion.extent.height = height;
	imageCopyRegion.extent.depth = 1;

	// Issue the copy command
	vkCmdCopyImage(
		copyCmd,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		screenImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);

	// Transition destination image to general layout, which is the required layout for mapping the image memory later on
	InsertImageMemoryBarrier(
		copyCmd,
		screenImage,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// Transition back the swap chain image after the blit is done
	InsertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	FVulkanCommandBufferCalculator::EndSingleTimeCommands(copyCmd, vulkanDevice.logicalDevice, queue, commandPool);

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkSubresourceLayout subResourceLayout;

	vkGetImageSubresourceLayout(vulkanDevice.logicalDevice, screenImage, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	vkMapMemory(vulkanDevice.logicalDevice, screenImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	std::ofstream file(filename, std::ios::out | std::ios::binary);

	// ppm header
	file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

	// ppm binary pixel data
	for (uint32_t y = 0; y < height; y++)
	{
		unsigned int *row = (unsigned int*)data;
		for (uint32_t x = 0; x < width; x++)
		{
			file.write((char*)row, 3);
			row++;
		}
		data += subResourceLayout.rowPitch;
	}
	file.close();

	std::cout << "Color Screenshot saved to disk" << std::endl;
}

void FVulkanScreenGrab::InsertImageMemoryBarrier(
	VkCommandBuffer cmdbuffer,
	VkImage image,
	VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask,
	VkImageSubresourceRange subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier = FVulkanInitializers::ImageMemoryBarrier();
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}
