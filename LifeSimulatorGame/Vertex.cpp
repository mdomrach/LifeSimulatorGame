#include "Vertex.h"
#include <vulkan/vulkan.h>
#include <glm/gtx/hash.hpp>

bool FVertex::operator==(const FVertex& Other) const
{
	return pos == Other.pos && color == Other.color && texCoord == Other.texCoord;
}


VkVertexInputBindingDescription FVertex::GetVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(FVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> FVertex::GetVertexAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> attributeDesciptions = {};

	attributeDesciptions[0].binding = 0;
	attributeDesciptions[0].location = 0;
	attributeDesciptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[0].offset = offsetof(FVertex, pos);

	attributeDesciptions[1].binding = 0;
	attributeDesciptions[1].location = 1;
	attributeDesciptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[1].offset = offsetof(FVertex, color);

	attributeDesciptions[2].binding = 0;
	attributeDesciptions[2].location = 2;
	attributeDesciptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDesciptions[2].offset = offsetof(FVertex, texCoord);

	return attributeDesciptions;
}
