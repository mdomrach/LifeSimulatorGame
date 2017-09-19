#pragma once

#include <vector>
#include <array>

using namespace std;

struct FTriangles
{
	vector<array<int, 3>> vertexIndices;
	vector<array<int, 3>> edgeIndices;
};