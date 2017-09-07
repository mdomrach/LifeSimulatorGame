#pragma once

#include <vector>

struct FTriangles
{
	int vertexIndicesCount;
	int (*vertexIndices)[3];
	int (*edgeIndices)[3];
};