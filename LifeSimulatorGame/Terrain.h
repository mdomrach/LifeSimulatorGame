#pragma once
#include <vector>

struct FTerrainVertex;

class FTerrain
{
public:
	std::vector<FTerrainVertex> vertices;
	std::vector<uint32_t> indices;
};