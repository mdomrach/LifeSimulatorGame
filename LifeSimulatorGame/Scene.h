#pragma once

class FMesh;
class FCamera;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class FScene
{
public:
	FMesh* mesh;
	glm::vec3 position;

	FCamera* camera;
};

