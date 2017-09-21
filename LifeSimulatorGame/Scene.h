#pragma once

class FMesh;
class FCamera;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vector>

class FScene
{
public:
	glm::vec3 cursor3Dposition;
	int cursor3DIndex;

	std::vector<FMesh*> displayedMeshes;
	
	FCamera* camera;
};

