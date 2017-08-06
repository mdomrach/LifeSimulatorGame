#pragma once

#include <glm/glm.hpp>
#include <array>

struct FVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const FVertex& Other) const;
	
	static struct VkVertexInputBindingDescription GetVertexBindingDescription();
	static std::array<struct VkVertexInputAttributeDescription, 3> GetVertexAttributeDescriptions();
};

#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<FVertex> {
		size_t operator()(FVertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}