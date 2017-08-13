#pragma once

#include <glm/glm.hpp>
#include <array>

struct FTerrainVertex
{
	glm::vec3 pos;
	glm::vec3 normal;

	static struct VkVertexInputBindingDescription GetVertexBindingDescription();
	static std::array<struct VkVertexInputAttributeDescription, 2> GetVertexAttributeDescriptions();
};