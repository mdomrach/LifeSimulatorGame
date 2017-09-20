#pragma once
#include <vector>

struct FMeshVertex;

class FMesh
{
public:
	std::vector<FMeshVertex> vertices;
	std::vector<uint32_t> indices;
};