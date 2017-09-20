#include "MeshVertex.h"
#include <vulkan/vulkan.h>
#include <glm/gtx/hash.hpp>

VkVertexInputBindingDescription FMeshVertex::GetVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(FMeshVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> FMeshVertex::GetVertexAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDesciptions = {};

	attributeDesciptions[0].binding = 0;
	attributeDesciptions[0].location = 0;
	attributeDesciptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[0].offset = offsetof(FMeshVertex, pos);

	attributeDesciptions[1].binding = 0;
	attributeDesciptions[1].location = 1;
	attributeDesciptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[1].offset = offsetof(FMeshVertex, normal);

	return attributeDesciptions;
}
