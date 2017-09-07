#include "TerrainCalculator.h"


int FTerrainCalculator::GetVertexIndex(int x, int y)
{
	return x * numberOfVertices + y;
}


int FTerrainCalculator::GetTriangleIndex(int xGrid, int yGrid)
{
	return 2 * (xGrid * numberOfQuads + yGrid);
}

//
//int FTerrainCalculator::GetTriangleIndex(float xWorld, float yWorld)
//{
//	float sizeOfQuads = 1.0f / (float) numberOfQuads;
//	int xGrid = xWorld / sizeOfQuads;
//	int yGrid = yWorld / sizeOfQuads;
//
//	float xLocal = xWorld - xGrid;
//
//
//	int quadIndex = 2 * (xGrid * numberOfQuads + yGrid);
//
//}