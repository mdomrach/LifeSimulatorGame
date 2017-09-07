#pragma once

#include <vector>

struct FVertices
{
	std::vector<int> (*triangleIndices);
	std::vector<int> (*edgeIndices);
};
