#include "BufferCreateInfo.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanInitializers.h"

VkResult FBufferCreateInfo::Create(const FVulkanDevice& vulkanDevice, FVulkanBuffer& buffer)
{
	VkBufferCreateInfo bufferInfo = FVulkanInitializers::BufferCreateInfo();
	bufferInfo.size = buffersize;
	bufferInfo.usage = bufferUsageFlags;
	bufferInfo.sharingMode = sharingMode;

	if (vkCreateBuffer(vulkanDevice.logicalDevice, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vulkanDevice.logicalDevice, buffer.buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = FVulkanInitializers::MemoryAllocateInfo();
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(vulkanDevice.physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	if (vkAllocateMemory(vulkanDevice.logicalDevice, &allocInfo, nullptr, &buffer.bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(vulkanDevice.logicalDevice, buffer.buffer, buffer.bufferMemory, 0);
	return VkResult::VK_SUCCESS;
}

uint32_t FBufferCreateInfo::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}