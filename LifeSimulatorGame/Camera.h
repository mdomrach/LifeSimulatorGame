#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class FCamera
{
public:
	FCamera(int width, int height);
	glm::mat4x4 view;
	glm::mat4x4 proj;

	glm::vec3 position;
	glm::quat rotation;
};

