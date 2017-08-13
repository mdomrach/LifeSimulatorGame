#include "TerrainVertex.h"
#include <vulkan/vulkan.h>
#include <glm/gtx/hash.hpp>

VkVertexInputBindingDescription FTerrainVertex::GetVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(FTerrainVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> FTerrainVertex::GetVertexAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDesciptions = {};

	attributeDesciptions[0].binding = 0;
	attributeDesciptions[0].location = 0;
	attributeDesciptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[0].offset = offsetof(FTerrainVertex, pos);

	attributeDesciptions[1].binding = 0;
	attributeDesciptions[1].location = 1;
	attributeDesciptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[1].offset = offsetof(FTerrainVertex, normal);

	return attributeDesciptions;
}
