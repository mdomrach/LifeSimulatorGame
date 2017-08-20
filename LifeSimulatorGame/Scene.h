#pragma once

class FMesh;
class FCamera;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class FScene
{
public:
	FMesh* mesh;
	FMesh* mesh2;
	glm::vec3 position;

	FCamera* camera;
};

