#pragma once
//#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#define PARTICLE_SIZE 10.0f

struct FUniformBufferObjectParticle
{
public:
	glm::mat4 projection;
	glm::mat4 model;
	glm::vec2 viewportDim;
	float pointSize = PARTICLE_SIZE;
};
