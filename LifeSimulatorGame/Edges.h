#pragma once

#include <vector>
#include <array>

using namespace std;

struct FEdges
{
	vector<array<int, 2>> vertexIndices;
	vector<int> triangleIndices;
};