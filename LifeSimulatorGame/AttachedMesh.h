#pragma once

#include <vector>
#include "AttachedVertex.h"
#include "MeshVertex.h"
#include "Mesh.h"

struct FDisplayedTerrain
{
	std::vector<int> copiedVertices;
	std::vector<FAttachedVertex<2>> edgeAttached;
	std::vector<FAttachedVertex<3>> faceAttached;
	
	FMesh Mesh;
};

