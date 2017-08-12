#pragma once

#include <glm/glm.hpp>
#include <array>

struct FTerrainVertex
{
	glm::vec3 pos;

	static struct VkVertexInputBindingDescription GetVertexBindingDescription();
	static std::array<struct VkVertexInputAttributeDescription, 1> GetVertexAttributeDescriptions();
};