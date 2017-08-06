#include "VulkanBufferCalculator.h"
#include "VulkanInitializers.h"
#include <stdexcept>
#include "VulkanCalculator.h"
#include "VulkanDevice.h"
#include "VulkanCommandBufferCalculator.h"

void FVulkanBufferCalculator::CreateBuffer(FVulkanDevice vulkanDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = FVulkanInitializers::BufferCreateInfo();
	bufferInfo.size = deviceSize;
	bufferInfo.usage = bufferUsageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vulkanDevice.logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vulkanDevice.logicalDevice, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = FVulkanInitializers::MemoryAllocateInfo();
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FVulkanCalculator::FindMemoryType(vulkanDevice.physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	if (vkAllocateMemory(vulkanDevice.logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(vulkanDevice.logicalDevice, buffer, bufferMemory, 0);
}

void FVulkanBufferCalculator::CopyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = FVulkanCommandBufferCalculator::BeginSingleTimeCommands(logicalDevice, commandPool);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

	FVulkanCommandBufferCalculator::EndSingleTimeCommands(commandBuffer, logicalDevice, queue, commandPool);
}
