#include "VulkanBuffer.h"

void FVulkanBuffer::Destroy(VkDevice device)
{
	if (buffer)
	{
		vkDestroyBuffer(device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (bufferMemory)
	{
		vkFreeMemory(device, bufferMemory, nullptr);
		bufferMemory = VK_NULL_HANDLE;
	}
}
