#pragma once
#include <vector>

struct FTerrainVertex;

class FMesh
{
public:
	std::vector<FTerrainVertex> vertices;
	std::vector<uint32_t> indices;
};

