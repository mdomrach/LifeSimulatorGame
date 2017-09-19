#pragma once

class FTerrainCalculator
{
public:
	static const int numberOfQuads = 16;
	static const int numberOfVertices = 17;

	static int GetVertexIndex(int xGrid, int yGrid);
	static int GetTriangleIndex(int xGrid, int yGrid);

	//static void CalculateTerrainDisplayMesh(const FVisibleTerainAreas& visibleTerrainAreas, const FTerrain& terrain, FTerrainDisplayMesh& terrainDisplayMesh);
};

