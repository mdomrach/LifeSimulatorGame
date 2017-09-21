#include "MeshCalculator.h"

void FMeshCalculator::AddQuad(int index1, int index2, int index3, int index4, std::vector<uint32_t>& meshIndices)
{
	meshIndices.push_back(index1);
	meshIndices.push_back(index2);
	meshIndices.push_back(index3);

	meshIndices.push_back(index4);
	meshIndices.push_back(index3);
	meshIndices.push_back(index2);
}