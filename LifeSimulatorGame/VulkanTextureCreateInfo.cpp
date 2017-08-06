#include "VulkanTextureCreateInfo.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VulkanDevice.h"
#include "VulkanBufferCalculator.h"
#include "VulkanImageCalculator.h"
#include "VulkanTexture.h"
#include "VulkanInitializers.h"
#include "VulkanFactory.h"

FVulkanTextureCreateInfo::FVulkanTextureCreateInfo()
{
	//imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
	//imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

VkResult FVulkanTextureCreateInfo::Create(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue, FVulkanTexture& texture)
{
	CreateImage(vulkanDevice, commandPool, queue, texture);
	CreateSampler(vulkanDevice, texture);
	CreateImageView(vulkanDevice, texture);

	return VK_SUCCESS;
}
void FVulkanTextureCreateInfo::CreateImage(FVulkanDevice vulkanDevice, VkCommandPool commandPool, VkQueue queue, FVulkanTexture& texture)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags stagingMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	FVulkanBufferCalculator::CreateBuffer(vulkanDevice, imageSize, stagingBufferUsageFlags, stagingMemoryPropertyFlags, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(vulkanDevice.logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vulkanDevice.logicalDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	VkImageUsageFlags textureUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	FVulkanImageCalculator::CreateImage(vulkanDevice, texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, textureUsageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.imageMemory);

	FVulkanImageCalculator::TransitionImageLayout(
		vulkanDevice.logicalDevice, commandPool, queue,
		texture.image, format, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	FVulkanImageCalculator::CopyImageToBuffer(
		vulkanDevice.logicalDevice, commandPool, queue,
		stagingBuffer, texture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	FVulkanImageCalculator::TransitionImageLayout(
		vulkanDevice.logicalDevice, commandPool, queue,
		texture.image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(vulkanDevice.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(vulkanDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void FVulkanTextureCreateInfo::CreateImageView(FVulkanDevice vulkanDevice,  FVulkanTexture& texture)
{
	texture.imageView = FVulkanFactory::ImageView(vulkanDevice.logicalDevice, texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}


void FVulkanTextureCreateInfo::CreateSampler(FVulkanDevice vulkanDevice, FVulkanTexture& texture)
{
	VkSamplerCreateInfo samplerInfo = FVulkanInitializers::SamplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(vulkanDevice.logicalDevice, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}
