#pragma once

#include <vector>

class FMesh;

class FMeshCalculator
{
public:
	static void FMeshCalculator::AddQuad(int index1, int index2, int index3, int index4, std::vector<uint32_t>& meshIndices);
};

