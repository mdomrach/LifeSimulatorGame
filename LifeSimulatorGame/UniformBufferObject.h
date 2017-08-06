#pragma once
//#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct FUniformBufferObject
{
public:
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};