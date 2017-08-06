#pragma once
#include <vector>

struct FVertex;

class FMesh
{
public:
	std::vector<FVertex> vertices;
	std::vector<uint32_t> indices;
};

