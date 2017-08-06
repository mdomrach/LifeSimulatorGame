#pragma once
#include <vector>

struct FVertex;

class FTerrain
{
public:
	std::vector<FVertex> vertices;
	std::vector<uint32_t> indices;
};

