#pragma once

#include <glm/glm.hpp>
#include <array>

#define VERTEX_BUFFER_BIND_ID 0

#define PARTICLE_TYPE_FLAME 0
#define PARTICLE_TYPE_SMOKE 1


class FParticle {
public:
	glm::vec4 pos;
	glm::vec4 color;
	float alpha;
	float size;
	float rotation;
	uint32_t type;
	// Attributes not used in shader
	glm::vec4 vel;
	float rotationSpeed;

	static struct VkVertexInputBindingDescription GetVertexBindingDescription();
	static std::array<struct VkVertexInputAttributeDescription, 6> GetVertexAttributeDescriptions();
};
