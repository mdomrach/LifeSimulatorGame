#include "VulkanCommandBufferCalculator.h"
#include "VulkanInitializers.h"

VkCommandBuffer FVulkanCommandBufferCalculator::BeginSingleTimeCommands(VkDevice logicalDevice, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = FVulkanInitializers::CommandBufferAllocateInfo();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = FVulkanInitializers::CommandBufferBeginInfo();
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void FVulkanCommandBufferCalculator::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice logicalDevice, VkQueue queue, VkCommandPool commandPool)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = FVulkanInitializers::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}
