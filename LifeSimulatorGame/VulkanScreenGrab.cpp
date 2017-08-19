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
#include "Scene.h"
#include "Camera.h"

//#define GLM_FORCE_RADIANS
//#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

void FVulkanScreenGrab::Initialize(FGameManager* gameManager)
{
	inputManager = gameManager->inputManager;
	vulkanApplication = gameManager->vulkanApplication;
	scene = gameManager->scene;

	inputManager->MonitorKeyState(GLFW_KEY_P);
	isScreenShot = false;
	data = nullptr;
	writeDepthToFile = false;
}

void FVulkanScreenGrab::ProcessInput()
{
	MapMemory(vulkanApplication->vulkanDevice);
	OutputCurrentMousePosDepth(vulkanApplication->swapChain);
}

void FVulkanScreenGrab::OutputCurrentMousePosDepth(FVulkanSwapChain swapChain)
{
	if (data == nullptr)
		return;

	auto width = swapChain.extent.width;
	auto height = swapChain.extent.height;
	auto currentXMousePos = inputManager->currentXMousePos;
	auto currentYMousePos = inputManager->currentYMousePos;

	if (currentXMousePos < 0 || currentXMousePos > width)
	{
		return;
	}
	if (currentYMousePos < 0 || currentYMousePos > height)
	{
		return;
	}

	const char* depths = data;
	depths += subResourceLayout.offset;

	auto elementSize = sizeof(float);
	uint32_t offsetToCurrentMousePos = currentYMousePos * subResourceLayout.rowPitch + currentXMousePos * elementSize;
	depths += offsetToCurrentMousePos;
	float *currentDepth = (float*)depths;
	auto temp = *currentDepth;
	//std::cout << temp << " " << currentYMousePos << " " << currentXMousePos << " " << offsetToCurrentMousePos << std::endl;
	//std::cout << temp << std::endl;

	//auto screenPosition = glm::vec3(inputManager->currentXMousePos / 800, 1 - inputManager->currentYMousePos / 600, temp);
	auto screenPosition = glm::vec3(inputManager->currentXMousePos, 600-inputManager->currentYMousePos, temp);
	auto worldPosition = glm::unProject(screenPosition, scene->camera->view, scene->camera->proj, glm::vec4(0, 0, 800, 600));
	//auto worldPosition = UnProject(screenPosition, scene->camera->view, scene->camera->proj);

	//auto worldPosition = glm::inverse(scene->camera->view) * glm::inverse(scene->camera->proj) * screenPosition;

		//gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	auto worldXPosition = (inputManager->currentXMousePos / 800) * 16 - 8;
	auto worldYPosition = (inputManager->currentYMousePos / 600) * -16 + 8;
	inputManager->HitPoint.x = worldXPosition;
	inputManager->HitPoint.y = worldYPosition;
	inputManager->HitPoint.z = 0;

	inputManager->HitPoint.x = worldPosition.x;
	inputManager->HitPoint.y = worldPosition.y;
	inputManager->HitPoint.z = worldPosition.z;
	//inputManager->HitPoint.z = 0;

	std::cout << "Screen1: " << inputManager->currentXMousePos << " " << inputManager->currentYMousePos << std::endl;
	std::cout << "Screen2: " << worldXPosition << " " << worldYPosition << std::endl;
	std::cout << "World:   " << worldPosition.x << " " << worldPosition.y << " " << worldPosition.z << std::endl;
}

glm::vec3 FVulkanScreenGrab::UnProject
(
	glm::vec3 const & win,
	glm::mat4 const & model,
	glm::mat4  const & proj
)
{
	glm::mat4 Inverse = inverse(proj * model);

	glm::vec4 tmp = glm::vec4(win, 1);
#		if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
	tmp.x = tmp.x * static_cast<T>(2) - static_cast<T>(1);
	tmp.y = tmp.y * static_cast<T>(2) - static_cast<T>(1);
#		else
	tmp = tmp * 2.0f - 1.0f;
#		endif

	glm::vec4 obj = Inverse * tmp;
	obj /= obj.w;

	return glm::vec3(obj);
}

void FVulkanScreenGrab::MapMemory(FVulkanDevice vulkanDevice)
{
	if (data != nullptr || !writeDepthToFile)
		return;

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{};
	subResource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	//VkSubresourceLayout subResourceLayout;

	vkGetImageSubresourceLayout(vulkanDevice.logicalDevice, screenImage, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	vkMapMemory(vulkanDevice.logicalDevice, screenImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	writeDepthToFile = false;
}

void FVulkanScreenGrab::WriteDepthToFile(FVulkanDevice vulkanDevice, FVulkanSwapChain swapChain, const char* filename)
{
	auto width = swapChain.extent.width;
	auto height = swapChain.extent.height;

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

	//std::cout << "Depth Screenshot saved to disk" << std::endl;
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

void FVulkanScreenGrab::CreateCommandBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, FVulkanSwapChain swapChain)
{
	// Images
	// Create the linear tiled destination image to copy to and to read the memory from
	VkImageCreateInfo imgCreateInfo = FVulkanInitializers::ImageCreateInfo();// (vks::initializers::imageCreateInfo());
	imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
	imgCreateInfo.format = FVulkanCalculator::FindDepthFormat(vulkanDevice.physicalDevice);
	imgCreateInfo.extent.width = swapChain.extent.width;
	imgCreateInfo.extent.height = swapChain.extent.height;
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

	// Command Buffers
	cmdBuffers.resize(swapChain.imageCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)cmdBuffers.size();

	if (vkAllocateCommandBuffers(vulkanDevice.logicalDevice, &commandBufferAllocateInfo, cmdBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void FVulkanScreenGrab::BuildCommandBuffers(FVulkanDevice vulkanDevice, VkCommandPool commandPool, FVulkanSwapChain swapChain, VkImage srcImage, VkQueue queue)
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = FVulkanInitializers::CommandBufferBeginInfo();

	for (int32_t i = 0; i < cmdBuffers.size(); ++i)
	{
		vkBeginCommandBuffer(cmdBuffers[i], &cmdBufferBeginInfo);

		VkImageMemoryBarrier imageMemoryBarrier = FVulkanInitializers::ImageMemoryBarrier();

		// Transition destination image to transfer destination layout
		InsertImageMemoryBarrier(
			cmdBuffers[i],
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
			cmdBuffers[i],
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
		imageCopyRegion.extent.width = swapChain.extent.width;
		imageCopyRegion.extent.height = swapChain.extent.height;
		imageCopyRegion.extent.depth = 1;

		// Issue the copy command
		vkCmdCopyImage(
			cmdBuffers[i],
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			screenImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		InsertImageMemoryBarrier(
			cmdBuffers[i],
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
			cmdBuffers[i],
			srcImage,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });


		if (vkEndCommandBuffer(cmdBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to end command buffer!");
		}
	}
}


void FVulkanScreenGrab::Submit(VkQueue queue, uint32_t bufferindex)
{
	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	writeDepthToFile = true;
}

void FVulkanScreenGrab::Destroy(FVulkanDevice vulkanDevice)
{
	vkDestroyImage(vulkanDevice.logicalDevice, screenImage, nullptr);
	vkFreeMemory(vulkanDevice.logicalDevice, screenImageMemory, nullptr);
}